/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <memory>
#include <utility>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Actor/Character/Zombie/ZombieSpawner.h"
#include "Source/Actor/Stage/Core.h"
#include "Source/Actor/Stage/StageField.h"
#include "Source/Actor/Weapon/Handgun/Handgun.h"
#include "Source/Camera/PlayerCamera.h"
#include "Source/Manager/InGameInfoHub.h"
#include "Source/UI/HUD/CoreHP/CoreHPBarMenu.h"
#include "Source/UI/HUD/Handgun/HandgunAmmoMenu.h"
#include "Source/UI/HUD/PlayerHP/HPBarMenu.h"


namespace app
{
    namespace
    {
        /** プレイヤーのHPバーのレイアウトのjson */
        constexpr const char* HP_BAR_JSON_PATH = "Assets/parameter/ui/HPBar.json";
        /** コアの耐久値バーのレイアウトのjson */
        constexpr const char* CORE_HP_BAR_JSON_PATH = "Assets/parameter/ui/CoreHPBar.json";
        /** ハンドガンの残段数UIのレイアウトのjson */
        constexpr const char* HANDGUN_AMMO_JSON_PATH = "Assets/parameter/ui/HandgunAmmo.json";
        /** レティクルのレイアウトのjson */
        constexpr const char* RETICLE_JSON_PATH = "Assets/parameter/ui/Reticle.json";
        /** コアを置く座標。コアの真下の地面(y=0)の座標を渡す。浮かせる高さはCore側で足す */
        const Vector3 CORE_POSITION(0.0f, 0.0f, 0.0f);
        /** Miroの初期座標 */
        const Vector3 MIRO_POSITION(100.0f, 100.0f, 0.0f);
        /** 空のキューブマップ(曇天の昼) */
        constexpr const wchar_t* SKY_CUBE_PATH = L"Assets/modelData/preset/skyCubeMapOvercast.dds";
        /** 空の明るさ(キューブマップの色に掛ける倍率) */
        constexpr float SKY_LUMINANCE = 1.0f;
    } // namespace


    DebugScene::DebugScene()
        : m_miro(nullptr)
        , m_playerCamera(nullptr)
        , m_hpBarPacket(nullptr)
        , m_handgun(nullptr)
        , m_ammoPacket(nullptr)
        , m_reticlePacket(nullptr)
        , m_coreHpBarPacket(nullptr)
    {
        // プレイヤーのHPバー。Miroより先に作って、InGameInfoHubに通知されたHPを受け取る
        ui::InitUIPacket(m_hpBarPacket, HP_BAR_JSON_PATH);
        if (ui::HPBarMenu* hpBarMenu = m_hpBarPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnPlayerHpChanged([hpBarMenu](const float hp, const float maxHp) { hpBarMenu->SetHp(hp, maxHp); });
        }

        // ハンドガンの残段数UI。ハンドガンより先に作って、InGameInfoHubに通知された残弾数を受け取る
        ui::InitUIPacket(m_ammoPacket, HANDGUN_AMMO_JSON_PATH);
        if (ui::HandgunAmmoMenu* ammoMenu = m_ammoPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnPlayerAmmoChanged([ammoMenu](const uint32_t remainingAmmo) { ammoMenu->SetAmmo(remainingAmmo); });
        }

        ui::InitUIPacket(m_reticlePacket, RETICLE_JSON_PATH);

        // コアの耐久値バー。コアより先に作って、InGameInfoHubに通知された耐久値を受け取る
        ui::InitUIPacket(m_coreHpBarPacket, CORE_HP_BAR_JSON_PATH);
        if (ui::CoreHPBarMenu* coreHpBarMenu = m_coreHpBarPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnCoreHpChanged([coreHpBarMenu](const float hp, const float maxHp) { coreHpBarMenu->SetHp(hp, maxHp); });
        }

        m_stageField = std::make_unique<actor::StageField>();
        m_core = std::make_unique<actor::Core>();
        m_core->SetPosition(CORE_POSITION);
        m_miro = std::make_unique<actor::Miro>();
        m_miro->SetPosition(MIRO_POSITION);
        m_zombieSpawner = std::make_unique<actor::ZombieSpawner>();
        m_handgun = std::make_unique<actor::Handgun>();
        m_playerCamera = std::make_unique<camera::PlayerCamera>();
    }


    DebugScene::~DebugScene()
    {
        // 登録した関数は、HPバーを参照している。HPバーが無くなる前に解除する
        InGameInfoHub::Get().ResetObservers();

        // 空はエンジンが持っているので、このシーンのものを描かなくする
        RenderingEngine::Get().ClearSkyCube();
    }


    bool DebugScene::Start()
    {
        RenderingEngine::Get().SetSkyCube(SKY_CUBE_PATH, SKY_LUMINANCE);

        m_miro->StartWrapper();
        m_handgun->StartWrapper();
        m_playerCamera->Start();

        // ゾンビのスポーナーに、コアの位置(地面の座標)を関数で教える。湧いたゾンビは、この位置へ向かう。
        // m_zombieSpawner は m_core より先に破棄されるので、破棄の間にこの関数は呼ばれない。
        m_zombieSpawner->SetCorePositionProvider([core = m_core.get()](Vector3& outPosition) {
            outPosition = core->GetTransform().m_position;
            return true;
        });
        // ゾンビのモデルなどを先に読み込んでおく。(最初に湧く時に、引っかからないように)
        m_zombieSpawner->Preload();

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

        // 最初の残弾数を通知
        InGameInfoHub::Get().NotifyPlayerAmmo(m_handgun->GetRemainingAmmo());
        return true;
    }


    void DebugScene::Update()
    {
        m_stageField->UpdateWrapper();
        m_core->UpdateWrapper();
        m_miro->UpdateWrapper();
        m_zombieSpawner->Update();
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

        // MiroがHPを通知した後に、HPバーを更新する
        m_hpBarPacket->Update();

        // コアの耐久値バー。カメラが更新された後に画面位置を求めて、コアが耐久値を通知した後に更新する
        if (ui::CoreHPBarMenu* coreHpBarMenu = m_coreHpBarPacket->GetMenu())
        {
            coreHpBarMenu->UpdateScreenPosition(m_core->GetHpBarWorldPosition());
        }
        m_coreHpBarPacket->Update();

        // ハンドガンが残弾数を通知した後に、残段数UIを更新する
        m_ammoPacket->Update();
        m_reticlePacket->Update();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
        m_stageField->RenderWrapper(rc);
        m_core->RenderWrapper(rc);
        m_miro->RenderWrapper(rc);
        m_zombieSpawner->Render(rc);
        m_handgun->RenderWrapper(rc);
    }


    void DebugScene::RenderUI(RenderContext& rc)
    {
        m_hpBarPacket->Render(rc);
        m_coreHpBarPacket->Render(rc);
        m_ammoPacket->Render(rc);
        m_reticlePacket->Render(rc);
    }


    bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }


    bool DebugScene::IsPausable() const
    {
        return true;
    }
} // namespace app
