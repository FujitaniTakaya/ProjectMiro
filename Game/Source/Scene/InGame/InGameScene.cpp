/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 */
#include "stdafx.h"

#include "InGameScene.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Actor/Character/Zombie/Spawn/ZombieSpawnParameter.h"
#include "Source/Actor/Character/Zombie/ZombieManager.h"
#include "Source/Actor/Stage/Core.h"
#include "Source/Actor/Stage/StageField.h"
#include "Source/Actor/Weapon/Handgun/Handgun.h"
#include "Source/Camera/PlayerCamera.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
#include "Source/Manager/InGameInfoHub.h"
#include "Source/MoveBehavior/ControllerSlot.h"
#include "Source/Navigation/NavDebugDraw.h"
#include "Source/Navigation/NavigationService.h"
#include "Source/Scene/Fade.h"
#include "Source/UI/HUD/Countdown/CountdownMenu.h"


namespace app
{
    namespace
    {
        /** カウントダウンの数字のレイアウトのjson */
        constexpr const char* COUNTDOWN_JSON_PATH = "Assets/parameter/ui/Countdown.json";
        /** フェードインが終わってから、数え始めるまでの待ち時間(秒) */
        constexpr float COUNTDOWN_START_DELAY = 1.0f;
        /** カウントダウンで最初に出す数(3なら 3,2,1 の順に出して、0でゲームを始める) */
        constexpr int32_t COUNTDOWN_COUNT = 3;
        /** カウントダウンの数字を切り替える間隔(秒) */
        constexpr float COUNTDOWN_INTERVAL = 1.0f;
        /** コアを置く座標。コアの真下の地面(y=0)の座標を渡す。浮かせる高さはCore側で足す */
        const Vector3 CORE_POSITION(0.0f, 0.0f, 0.0f);
        /** Miroの初期座標 */
        const Vector3 MIRO_POSITION(100.0f, 100.0f, 0.0f);
        /** 空のキューブマップ(曇天の昼) */
        constexpr const wchar_t* SKY_CUBE_PATH = L"Assets/modelData/preset/skyCubeMapOvercast.dds";
        /** 空の明るさ(キューブマップの色に掛ける倍率) */
        constexpr float SKY_LUMINANCE = 1.0f;
    } // namespace


    InGameScene::InGameScene()
        : m_hud()
        , m_miro(nullptr)
        , m_playerCamera(nullptr)
        , m_handgun(nullptr)
        , m_countdown(COUNTDOWN_START_DELAY, COUNTDOWN_COUNT, COUNTDOWN_INTERVAL)
        , m_countdownPacket(nullptr)
        , m_playerController(nullptr)
    {
        ui::InitUIPacket(m_countdownPacket, COUNTDOWN_JSON_PATH);

        m_stageField = std::make_unique<actor::StageField>();
        m_core = std::make_unique<actor::Core>();
        m_core->SetPosition(CORE_POSITION);
        m_miro = std::make_unique<actor::Miro>();
        m_miro->SetPosition(MIRO_POSITION);
        m_zombieManager = std::make_unique<actor::ZombieManager>();
        m_handgun = std::make_unique<actor::Handgun>();
        m_playerCamera = std::make_unique<camera::PlayerCamera>();
    }


    InGameScene::~InGameScene()
    {
        // InGameInfoHubの登録の解除は、InGameHudのデストラクタが行う

        // 空はエンジンが持っているので、このシーンのものを描かなくする
        RenderingEngine::Get().ClearSkyCube();

        // 経路探索の格子はこのシーンのステージから作ったもの
        nav::NavigationService::Get().Clear();
    }


    bool InGameScene::Start()
    {
        RenderingEngine::Get().SetSkyCube(SKY_CUBE_PATH, SKY_LUMINANCE);

        m_miro->StartWrapper();
        // カウントダウンの間は、Miroを操作させない。パッドのコントローラーを外して持っておき、0になったら付け直す
        m_playerController = m_miro->GetControllerSlot().Detach();
        m_handgun->StartWrapper();
        m_playerCamera->Start();

        // ゾンビのマネージャーに、コアの位置(地面の座標)を関数で教える。湧いたゾンビは、この位置へ向かう。
        // m_zombieManager は m_core より先に破棄されるので、破棄の間にこの関数は呼ばれない。
        m_zombieManager->SetCorePositionProvider([core = m_core.get()]() { return core->GetTransform().m_position; });
        // ゾンビの攻撃判定がコアの箱に重なったら、コアにダメージを与える。(コア以外に重なった時は、何もしない)
        // m_zombieManager は m_core より先に破棄されるので、破棄の間にこの関数は呼ばれない。
        m_zombieManager->SetCoreAttackHandler([core = m_core.get()](const btCollisionObject& contact, const float damage) {
            if (!core->IsHitBox(contact))
            {
                return false;
            }
            core->TakeDamage(damage);
            return true;
        });
        // ゾンビのモデルなどを先に読み込んでおく。(最初に湧く時に、引っかからないように)
        m_zombieManager->Preload();

        // ハンドガンをMiroの手に持たせる。ボーンはMiroのStart()でモデルをロードした後に取れる
        m_handgun->AttachToBone(m_miro->GetWeaponHandBone());

        // Miroの武器の処理を、ハンドガンに結びつける。
        // fire: 構えてZRを押した時。撃てたかどうか(弾切れ・リロード中は撃てない)をMiroに返す
        // canReload・isEmpty: リロードステートに入るかの判定。Xを押した時と、マガジンが空になった時に使う
        // reload: リロードステートに入った時
        actor::MiroWeaponHandler weaponHandler;
        weaponHandler.fire = [this]() { return m_handgun->Fire(); };
        weaponHandler.canReload = [this]() { return m_handgun->CanReload(); };
        weaponHandler.isEmpty = [this]() { return m_handgun->GetRemainingAmmo() == 0; };
        weaponHandler.reload = [this]() { m_handgun->Reload(); };
        m_miro->SetWeaponHandler(std::move(weaponHandler));

        // ステージのフィールド。ワールド行列は、ここで確定する
        m_stageField->StartWrapper();
        m_core->StartWrapper();

        // 背景の当たり判定。モデルの形をそのまま使うので、ワールド行列(Z-upの補正込み)が確定した後に作る
        ModelRender& stageModelRender = m_stageField->GetModelRender();
        m_bgPhysicsStaticObject.CreateFromModel(stageModelRender.GetModel(), stageModelRender.GetModel().GetWorldMatrix());

        // 経路探索の格子。同じくワールド行列が確定した後に作る。ゾンビが最初に歩き出すまでに必要
        nav::NavigationService::Get().Build(actor::StageField::GetModelPath(), stageModelRender.GetModel().GetWorldMatrix());

        // 最初の残弾数を通知
        InGameInfoHub::Get().NotifyPlayerAmmo(m_handgun->GetRemainingAmmo());
        return true;
    }


    void InGameScene::UpdateCountdown()
    {
        if (m_countdown.IsFinished())
        {
            return;
        }

        // フェードイン中(まだ画面が暗い間)は数えない。(カウントダウン中はポーズできないので、途中で止まることはない)
        if (!Fade::Get().IsFading())
        {
            m_countdown.Update(g_gameTime->GetFrameDeltaTime());
        }

        ui::CountdownMenu* countdownMenu = m_countdownPacket->GetMenu();
        if (m_countdown.IsFinished())
        {
            // 0になった:「Start」を出して、Miroを操作できるようにする。(「Start」は、メニューが一定の時間で自分で消す)
            m_miro->GetControllerSlot().Attach(std::move(m_playerController));
            if (countdownMenu)
            {
                countdownMenu->ShowStart();
            }
        }
        else if (countdownMenu && m_countdown.IsCounting())
        {
            countdownMenu->Show(m_countdown.GetNumber(), m_countdown.GetNumberElapsed());
        }
    }


    void InGameScene::Update()
    {
        UpdateCountdown();

        m_stageField->UpdateWrapper();
        m_core->UpdateWrapper();
        m_miro->UpdateWrapper();
        // カウントダウンの間は、ゾンビを湧かせない
        if (m_countdown.IsFinished())
        {
            m_zombieManager->Update();
        }
        m_handgun->UpdateWrapper();

        // TEMP: 残段数UIの確認用。確認が終わったら取り除く
#ifdef BALLOON_IMGUI_ENABLED
        if (ImGui::Begin("Handgun (TEMP)"))
        {
            ImGui::Text("ammo: %u / %u%s", m_handgun->GetRemainingAmmo(), m_handgun->GetMagazineSize(), m_handgun->IsReloading() ? " (reloading)" : "");
            if (ImGui::Button("Fire"))
            {
                m_handgun->Fire();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload"))
            {
                m_handgun->Reload();
            }
        }
        ImGui::End();
#endif // BALLOON_IMGUI_ENABLED

        // ハンドガンが更新された後の残弾数を通知する(値が変わった時だけ、UIに伝わる)
        InGameInfoHub::Get().NotifyPlayerAmmo(m_handgun->GetRemainingAmmo());

        // Miroが動いた後の座標を追う。構えている間は、エイム感度で回る
        m_playerCamera->SetTargetPosition(m_miro->GetTransform().m_position);
        m_playerCamera->SetAiming(m_miro->IsAiming());
        m_playerCamera->Update();

        // 経路探索のデバッグ表示。カメラが更新された後に描く。始点は4つのゲート、終点はコア
#ifdef BALLOON_IMGUI_ENABLED
        {
            const actor::ZombieSpawnParameter& spawnParameter = ParamHolder::Get().GetParameter<actor::ZombieSpawnParameter>(EnParamID::ZombieSpawn);
            std::vector<Vector3> gatePositions;
            for (uint8_t gate = 0; gate < static_cast<uint8_t>(actor::EnZombieGate::Max); ++gate)
            {
                gatePositions.push_back(actor::GetGatePosition(spawnParameter, static_cast<actor::EnZombieGate>(gate)));
            }
            nav::NavDebugDraw::Get().Draw(gatePositions, m_core->GetTransform().m_position);
        }
#endif // BALLOON_IMGUI_ENABLED

        // HUD。コアの耐久値バーの画面位置は、カメラが更新された後に求める。
        // Miro・コア・ハンドガンが値を通知した後に、更新する
        m_hud.SetCoreHpBarWorldPosition(m_core->GetHpBarWorldPosition());
        m_hud.Update();

        // カウントダウンの数字。この関数の先頭で、数字を設定してある
        m_countdownPacket->Update();
    }


    void InGameScene::PauseUpdate()
    {}


    void InGameScene::Render(RenderContext& rc)
    {
        m_stageField->RenderWrapper(rc);
        m_core->RenderWrapper(rc);
        m_miro->RenderWrapper(rc);
        m_zombieManager->Render(rc);
        m_handgun->RenderWrapper(rc);
    }


    void InGameScene::RenderUI(RenderContext& rc)
    {
        m_hud.Render(rc);
        m_countdownPacket->Render(rc);
    }


    bool InGameScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }


    bool InGameScene::IsPausable() const
    {
        // カウントダウンの間は、ポーズさせない
        return m_countdown.IsFinished();
    }
} // namespace app
