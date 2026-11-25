/**
 * @file InGameScene.cpp
 * @brief インゲームシーン
 */
#include "stdafx.h"

#include "InGameScene.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Actor/Character/Zombie/Spawn/ZombieSpawnParameter.h"
#include "Source/Actor/Character/Zombie/ZombieBase.h"
#include "Source/Actor/Character/Zombie/ZombieManager.h"
#include "Source/Actor/Stage/Core.h"
#include "Source/Actor/Stage/StageBoundary.h"
#include "Source/Actor/Stage/StageField.h"
#include "Source/Actor/Weapon/Bullet/BulletManager.h"
#include "Source/Actor/Weapon/Bullet/BulletParameter.h"
#include "Source/Actor/Weapon/Bullet/BulletSweep.h"
#include "Source/Actor/Weapon/Handgun/Handgun.h"
#include "Source/Camera/AimAssistParameter.h"
#include "Source/Camera/PlayerCamera.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Manager/InGameInfoHub.h"
#include "Source/MoveBehavior/ControllerSlot.h"
#include "Source/Navigation/NavDebugDraw.h"
#include "Source/Navigation/NavigationService.h"
#include "Source/Scene/Fade.h"
#include "Source/Scene/ResultScene.h"
#include "Source/Score/GameResult.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/HUD/Countdown/CountdownMenu.h"
#include "Source/UI/HUD/GameSet/GameSetMenu.h"


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
        /** 「Finish」の文字のレイアウトのjson */
        constexpr const char* GAME_SET_JSON_PATH = "Assets/parameter/ui/GameSet.json";
        /** 「Finish」を出してから、フェードアウトを始めるまでの待ち時間(秒) */
        constexpr float FINISH_WAIT_TIME = 3.0f;
        /** リザルトへ進む時の、暗転・明転にかける時間(秒)。タイトルからインゲームへ進む時と同じ */
        constexpr float RESULT_FADE_DURATION = 1.0f;
        /** インゲームBGMのフェードインにかける時間(秒)。「Start」を出した時から */
        constexpr float BGM_FADE_IN_SECONDS = 1.0f;
        /** 次のシーン。ゲームセットになったら、リザルトシーンに進む */
        constexpr uint32_t RESULT_SCENE_ID = ResultScene::ID();
        /** コアを置く座標。コアの真下の地面(y=0)の座標を渡す。浮かせる高さはCore側で足す */
        const Vector3 CORE_POSITION(0.0f, 0.0f, 0.0f);
        /** Miroの初期座標 */
        const Vector3 MIRO_POSITION(100.0f, 100.0f, 0.0f);
        /** 空のキューブマップ(曇天の昼) */
        constexpr const wchar_t* SKY_CUBE_PATH = L"Assets/modelData/preset/skyCubeMapOvercast.dds";
        /** 空の明るさ(キューブマップの色に掛ける倍率) */
        constexpr float SKY_LUMINANCE = 1.0f;
        /** Miroの足元から胸までの高さ(cm)。壁に銃口が出ていないかを、胸から銃口までの球で調べる時の始点(Miroの全高は160) */
        constexpr float CHEST_HEIGHT = 120.0f;
    } // namespace


    InGameScene::InGameScene()
        : m_hud()
        , m_miro(nullptr)
        , m_playerCamera(nullptr)
        , m_handgun(nullptr)
        , m_countdown(COUNTDOWN_START_DELAY, COUNTDOWN_COUNT, COUNTDOWN_INTERVAL)
        , m_lastCountdownNumber(0)
        , m_elapsedTime(0.0f)
        , m_isGameOver(false)
        , m_finishTimer()
        , m_countdownPacket(nullptr)
        , m_gameSetPacket(nullptr)
        , m_playerController(nullptr)
        , m_aimTargets()
    {
        m_finishTimer.SetTimeLimit(FINISH_WAIT_TIME);
        ui::InitUIPacket(m_countdownPacket, COUNTDOWN_JSON_PATH);
        ui::InitUIPacket(m_gameSetPacket, GAME_SET_JSON_PATH);

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

        // 弾はエンジンの資源(モデル)を持つので、シーンの終了時に片付ける
        actor::BulletManager::Get().Clear();

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

        // ゾンビのマネージャーに、向かう先の位置(地面の座標)を関数で教える。湧いたゾンビは、この位置へ向かう。
        // ロックオンしている(アンガー値がしきい値に達した)ゾンビは、Miroが生きている間、Miroへ向かう。それ以外は、コアへ向かう。
        // m_zombieManager は m_core・m_miro より先に破棄されるので、破棄の間にこの関数は呼ばれない。
        m_zombieManager->SetCorePositionProvider([core = m_core.get(), miro = m_miro.get()](const Vector3&, const bool isLockedOn) -> std::optional<Vector3> {
            if (isLockedOn && miro->GetHp() > 0.0f)
            {
                return miro->GetTransform().m_position;
            }
            return core->GetTransform().m_position;
        });
        // ゾンビの攻撃判定がコアの箱かMiroの体に重なったら、ダメージを与える。(それ以外に重なった時は、何もしない)
        // m_zombieManager は m_core・m_miro より先に破棄されるので、破棄の間にこの関数は呼ばれない。
        m_zombieManager->SetCoreAttackHandler([core = m_core.get(), miro = m_miro.get()](const btCollisionObject& contact, const float damage) {
            if (core->IsHitBox(contact))
            {
                core->TakeDamage(damage);
                return true;
            }
            if (miro->GetHp() > 0.0f && &contact == miro->GetCharacterController()->GetRigidBody()->GetBody())
            {
                miro->TakeDamage(damage);
                return true;
            }
            return false;
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
        weaponHandler.fire = [this]() { return FireHandgun(); };
        weaponHandler.canReload = [this]() { return m_handgun->CanReload(); };
        weaponHandler.isEmpty = [this]() { return m_handgun->GetRemainingAmmo() == 0; };
        weaponHandler.reload = [this]() { m_handgun->Reload(); };
        m_miro->SetWeaponHandler(std::move(weaponHandler));

        // 弾が当たった時の処理。当たったのがゾンビならダメージを与える。(ステージ・コアに当たった時は、弾が止まるだけ)
        // BulletManager::Clear() でこの処理は外れる。(シーンのデストラクタで呼ばれるので、this が無効になった後には呼ばれない)
        actor::BulletManager::Get().SetHitHandler([this](const actor::BulletHit& hit) {
            actor::ZombieBase* zombie = m_zombieManager->FindByBody(*hit.object);
            if (zombie == nullptr)
            {
                return;
            }
            // ダメージは毎回パラメーターから読む。(jsonのホットリロードがすぐ反映される)
            const actor::BulletParameter& bulletParameter = ParamHolder::Get().GetParameter<actor::BulletParameter>(EnParamID::Bullet);
            zombie->TakeShotDamage(bulletParameter.damage);
        });

        // ステージのフィールド。ワールド行列は、ここで確定する
        m_stageField->StartWrapper();
        m_core->StartWrapper();

        // 背景の当たり判定。モデルの形をそのまま使うので、ワールド行列(Z-upの補正込み)が確定した後に作る
        ModelRender& stageModelRender = m_stageField->GetModelRender();
        m_bgPhysicsStaticObject.CreateFromModel(stageModelRender.GetModel(), stageModelRender.GetModel().GetWorldMatrix());

        // Miroは、当たり判定ができる前に Start() している。浮かせて置いた座標から、足元の地面へ落とす
        m_miro->SnapToGround();

        // ステージの外へ出られないように、外周に見えない壁を置く
        m_stageBoundary.Build();

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
            SoundManager::Get().GetSE().Play(EnSoundID::SeGameStart);
            // 「Start」を出すのと同時に、インゲームBGMを始める。(カウントダウンの間は無音)ポーズ中も鳴らし続ける
            SoundManager::Get().GetBGM().Play(EnSoundID::BgmInGame, DEFAULT_VOLUME_MAGNIFICATION, BGM_FADE_IN_SECONDS);
        }
        else if (countdownMenu && m_countdown.IsCounting())
        {
            // 数字が切り替わった時に、1回だけ鳴らす
            if (m_countdown.GetNumber() != m_lastCountdownNumber)
            {
                m_lastCountdownNumber = m_countdown.GetNumber();
                SoundManager::Get().GetSE().Play(EnSoundID::SeCountdown);
            }
            countdownMenu->Show(m_countdown.GetNumber(), m_countdown.GetNumberElapsed());
        }
    }


    void InGameScene::UpdateGameOver()
    {
        // ゲームセットの後は、リザルトへ進むまでの待ち時間を進める。(「Finish」を出したフレームは数えず、次のフレームから数える)
        if (m_isGameOver)
        {
            m_finishTimer.Update(g_gameTime->GetFrameDeltaTime());
            return;
        }

        // コアの耐久値もMiroのHPも残っていれば続ける
        if (m_core->GetHp() > 0.0f && m_miro->GetHp() > 0.0f)
        {
            return;
        }

        m_isGameOver = true;
        // リザルトへ進むまでの待ち時間をかけて、BGMを消す。リザルトBGMは、ResultScene::Start()でフェードインする
        SoundManager::Get().GetBGM().FadeOutAll(FINISH_WAIT_TIME);
        // 結果をリザルトへ渡す。経過時間はここで止まるので、そのまま守衛継続時間になる
        GameResult::Get().Set(GameResultData(m_elapsedTime, m_zombieManager->GetKillCount(actor::EnZombieKind::Normal), m_zombieManager->GetKillCount(actor::EnZombieKind::Running)));
        // Miroを操作できなくする。(カウントダウンの時と同じく、パッドのコントローラーを外す)
        m_playerController = m_miro->GetControllerSlot().Detach();
        // 「Finish」を出す
        if (ui::GameSetMenu* gameSetMenu = m_gameSetPacket->GetMenu())
        {
            gameSetMenu->Show();
            SoundManager::Get().GetSE().Play(EnSoundID::SeGameFinish);
        }
    }


    bool InGameScene::FireHandgun()
    {
        // 弾切れ・リロード中は撃てない
        if (!m_handgun->Fire())
        {
            // 弾切れで引き金を引いた時だけ、空撃ちの音を鳴らす。(リロード中は鳴らさない)
            if (m_handgun->GetRemainingAmmo() == 0 && !m_handgun->IsReloading())
            {
                SoundManager::Get().GetSE().Play(EnSoundID::SePistolDryShot);
            }
            return false;
        }
        SoundManager::Get().GetSE().Play(EnSoundID::SePistolShot);
        // 撃った。レティクルが広がり、カメラが跳ね上がる
        InGameInfoHub::Get().NotifyPlayerFired();
        m_playerCamera->AddRecoil();

        Vector3 muzzle;
        Quaternion barrelRotation;
        if (!m_handgun->CalcMuzzle(muzzle, barrelRotation))
        {
            return true;
        }

        // パラメーターは、毎回読む。(jsonのホットリロードがすぐ反映される)
        const actor::BulletParameter& bulletParameter = ParamHolder::Get().GetParameter<actor::BulletParameter>(EnParamID::Bullet);
        // 弾が自分に当たらないように、Miroの体を無視する
        const btCollisionObject* selfBody = m_miro->GetCharacterController()->GetRigidBody()->GetBody();

        // 壁際で銃口が壁やゾンビの向こう側に出ていたら、弾を出さず、その場で着弾にする。(弾の最初の位置が壁の中だと、壁を抜けてしまう)
        const Vector3 chest = m_miro->GetTransform().m_position + Vector3(0.0f, CHEST_HEIGHT, 0.0f);
        actor::BulletHit blockedHit;
        if (actor::SweepSphere(chest, muzzle, bulletParameter.hitRadius, selfBody, blockedHit))
        {
            actor::BulletManager::Get().NotifyHit(blockedHit);
            return true;
        }

        // レティクル(カメラの視線)が指す点へ撃つ。視線の長さは、弾が飛べる距離(寿命 × 速さ)
        const Quaternion rotation = actor::CalcAimRotation(
            g_camera3D->GetPosition(),
            g_camera3D->GetForward(),
            muzzle,
            bulletParameter.lifeTime * bulletParameter.shotSpeed,
            bulletParameter.hitRadius,
            selfBody,
            barrelRotation
        );
        actor::BulletManager::Get().Shot(rotation, muzzle, bulletParameter.shotSpeed, selfBody);
        // 銃口の発砲炎。弾と同じ向きに出す。(エフェクトの+Zが向き)
        EffectManager::Get().PlayEffect(EnEffectKind::MuzzleFlash, muzzle, rotation, Vector3::One);
        return true;
    }


    bool InGameScene::IsAimingAtZombie() const
    {
        // 撃てるのは、構えを保持している間だけ
        if (!m_miro->IsAimHeld())
        {
            return false;
        }

        Vector3 muzzle;
        Quaternion barrelRotation;
        if (!m_handgun->CalcMuzzle(muzzle, barrelRotation))
        {
            return false;
        }

        // 弾が狙う点と同じ視線で調べる。(見た目と、弾の狙いを一致させる)
        const actor::BulletParameter& bulletParameter = ParamHolder::Get().GetParameter<actor::BulletParameter>(EnParamID::Bullet);
        const btCollisionObject* selfBody = m_miro->GetCharacterController()->GetRigidBody()->GetBody();
        actor::BulletHit hit;
        if (!actor::SweepAimRay(
                g_camera3D->GetPosition(),
                g_camera3D->GetForward(),
                muzzle,
                bulletParameter.lifeTime * bulletParameter.shotSpeed,
                bulletParameter.hitRadius,
                selfBody,
                hit
            ))
        {
            return false;
        }

        // 死んだゾンビ(死亡アニメーション中)は、撃っても意味がないので数えない
        const actor::ZombieBase* zombie = m_zombieManager->FindByBody(*hit.object);
        return zombie != nullptr && !zombie->IsDead();
    }


    void InGameScene::Update()
    {
        UpdateCountdown();

        m_stageField->UpdateWrapper();
        m_core->UpdateWrapper();
        m_miro->UpdateWrapper();
        // 前のフレームまでのゾンビの攻撃・Miroの更新でHPが0になっていたら、ゲームセットにする
        UpdateGameOver();
        // カウントダウンの間とゲームセット後は、ゾンビを湧かせず(動かさず)、経過時間も進めない。
        // (ポーズ中は Update() が呼ばれないので、止まる)
        if (m_countdown.IsFinished() && !m_isGameOver)
        {
            m_elapsedTime += g_gameTime->GetFrameDeltaTime();
            m_zombieManager->Update();
        }
        m_handgun->UpdateWrapper();
        // 弾を進める。(ポーズ中は Update() が呼ばれないので、弾も止まる)
        actor::BulletManager::Get().Update();

        // TEMP: 残段数UIの確認用。確認が終わったら取り除く
#ifdef BALLOON_IMGUI_ENABLED
        if (ImGui::Begin("Handgun (TEMP)"))
        {
            ImGui::Text("ammo: %u / %u%s", m_handgun->GetRemainingAmmo(), m_handgun->GetMagazineSize(), m_handgun->IsReloading() ? " (reloading)" : "");
            if (ImGui::Button("Fire"))
            {
                FireHandgun();
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

        // 構えを通知する(値が変わった時だけ、UIに伝わる)。ZLを押している間だけ構え扱いで、レティクルは中央に集まる。
        // 離した瞬間から広がり始める(構えを解く動きの終わりは待たない)
        InGameInfoHub::Get().NotifyPlayerAim(m_miro->IsAimHeld());

        // Miroが動いた後の座標を追う。ZLを押している間は、エイム感度で回り、カメラが近づく。
        // 離した瞬間から、カメラも引き始める
        m_playerCamera->SetTargetPosition(m_miro->GetTransform().m_position);
        m_playerCamera->SetAiming(m_miro->IsAimHeld());
        // エイムアシストの対象。構えている間だけ、生きているゾンビの胸の座標を渡す
        if (m_miro->IsAimHeld())
        {
            const camera::AimAssistParameter& assistParameter = ParamHolder::Get().GetParameter<camera::AimAssistParameter>(EnParamID::AimAssist);
            m_zombieManager->CollectAimTargets(m_aimTargets, assistParameter.targetHeight);
            m_playerCamera->SetAimTargets(m_aimTargets);
        }
        m_playerCamera->Update();
        // カメラが更新された後の視線で、レティクルがゾンビを指しているか調べる
        m_hud.SetReticleOnTarget(IsAimingAtZombie());

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
        m_hud.SetElapsedTime(m_elapsedTime);
        m_hud.Update();

        // カウントダウンの数字。この関数の先頭で、数字を設定してある
        m_countdownPacket->Update();
        m_gameSetPacket->Update();
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
        actor::BulletManager::Get().Render(rc);
    }


    void InGameScene::RenderUI(RenderContext& rc)
    {
        m_hud.Render(rc);
        m_countdownPacket->Render(rc);
        m_gameSetPacket->Render(rc);
    }


    bool InGameScene::RequestScene(uint32_t& id, float& waitTime)
    {
        // ゲームセットになって、「Finish」を出してから一定の時間が経ったら、リザルトへ進む
        if (!m_isGameOver || !m_finishTimer.IsTimeUp())
        {
            return false;
        }

        id = RESULT_SCENE_ID;
        waitTime = RESULT_FADE_DURATION;
        return true;
    }


    bool InGameScene::IsPausable() const
    {
        // カウントダウンの間は、ポーズさせない
        return m_countdown.IsFinished();
    }
} // namespace app
