/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>

// 一時的にコメントアウト(Miroを使わない)
// #include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Actor/Character/Zombie/NormalZombie/NormalZombie.h"
#include "Source/Actor/Character/Zombie/RunningZombie/RunningZombie.h"
#include "Source/Actor/Character/Zombie/RunningZombie/RunningZombieController.h"
#include "Source/Actor/Character/Zombie/ZombieBase.h"
#include "Source/Actor/Stage/StageField.h"
#include "Source/Actor/Weapon/Bullet/BulletManager.h"
// #include "Source/Actor/Weapon/Bullet/BulletParameter.h"
// #include "Source/Actor/Weapon/Bullet/BulletSweep.h"
// #include "Source/Actor/Weapon/Handgun/Handgun.h"
#include "Source/Camera/FreeCamera.h"
// #include "Source/Controller/PlayerController.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
// #include "Source/Effect/EffectManager.h"
#include "Source/MoveBehavior/ControllerSlot.h"
#include "Source/MoveBehavior/IController.h"


namespace app
{
    namespace
    {
        /** 空のキューブマップ(曇天の昼)。InGameScene と同じ */
        constexpr const wchar_t* SKY_CUBE_PATH = L"Assets/modelData/preset/skyCubeMapOvercast.dds";
        /** 空の明るさ(キューブマップの色に掛ける倍率) */
        constexpr float SKY_LUMINANCE = 1.0f;
        // 一時的にコメントアウト(Miroを使わない)
        // /** Miroの初期座標。InGameScene と同じ */
        // const Vector3 MIRO_POSITION(100.0f, 100.0f, 0.0f);
        // /** Miroの足元から胸までの高さ(cm)。壁に銃口が出ていないかを、胸から銃口までの球で調べる時の始点(Miroの全高は160)。InGameScene と同じ */
        // constexpr float CHEST_HEIGHT = 120.0f;
        /** 入力を読み取るパッドの番号 */
        constexpr int PAD_INDEX = 0;

        /** ゾンビの集まりの中心。カメラが追う点で、走るゾンビが向かう先でもある(足元の高さ) */
        const Vector3 GROUP_CENTER(100.0f, 0.0f, 0.0f);
        /** 地面へ落とすために、足元の座標から浮かせる高さ(cm) */
        constexpr float DROP_HEIGHT = 100.0f;

        /** 普通のゾンビの人数 */
        constexpr size_t NORMAL_ZOMBIE_COUNT = 3;
        /** 普通のゾンビの位置。GROUP_CENTER からのずれ(XZ平面、cm)。3体が、少し離れて並ぶ */
        const std::array<Vector3, NORMAL_ZOMBIE_COUNT> NORMAL_ZOMBIE_OFFSETS = {
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(-120.0f, 0.0f, 150.0f),
            Vector3(-120.0f, 0.0f, -150.0f),
        };
        /** 普通のゾンビが向く方向(XZ平面)。カメラの向きに合わせて変える */
        const Vector3 NORMAL_ZOMBIE_FACING(1.0f, 0.0f, 0.0f);
        /** 普通のゾンビが攻撃アニメーションを出す間隔(秒) */
        constexpr float ATTACK_INTERVAL_SECONDS = 4.0f;
        /** 普通のゾンビごとの、最初の攻撃までのずらし(秒)。同時に振らないように、順にずらす */
        constexpr float ATTACK_FIRST_DELAY_STEP_SECONDS = 1.3f;
        /** 攻撃ボタンを押している時間(秒)。攻撃のステートに入れる長さ(動きの再生は、ボタンを離しても最後まで続く) */
        constexpr float ATTACK_PRESS_SECONDS = 0.1f;

        /** 走るゾンビの人数 */
        constexpr int RUNNING_ZOMBIE_COUNT = 4;
        /** 走るゾンビを置く、GROUP_CENTER からの距離(cm)。ここから、GROUP_CENTER へ走る */
        constexpr float RUNNING_ZOMBIE_DISTANCE = 1200.0f;
        /** 走るゾンビを置く方向の最初の角度(度)。人数で等分して並べる */
        constexpr float RUNNING_ZOMBIE_FIRST_ANGLE_DEG = 35.0f;
        /** 走るゾンビの角度を、人数の等分からずらす量(度)。一直線に並ばないように、交互にずらす */
        constexpr float RUNNING_ZOMBIE_ANGLE_JITTER_DEG = 12.0f;
        /** 円周率 */
        constexpr float PI = 3.14159265f;


        /**
         * @brief 何秒かおきに、攻撃ボタンを押すController
         * @details 動かず(移動入力なし)、その場で一定の間隔で攻撃のアニメーションを出す。スクリーンショットの撮影用。
         */
        class PeriodicAttackController : public IController
        {
        public:
            /**
             * @brief コンストラクタ
             * @param intervalSeconds 攻撃する間隔(秒)
             * @param firstDelaySeconds 最初の攻撃までの時間(秒)
             */
            PeriodicAttackController(const float intervalSeconds, const float firstDelaySeconds)
                : m_interval(intervalSeconds)
                , m_timeToNextAttack(firstDelaySeconds)
                , m_pressRemain(0.0f)
            {
            }


            CharacterInput Update(const ControllerContext& context) override
            {
                m_timeToNextAttack -= context.deltaTime;
                if (m_timeToNextAttack <= 0.0f)
                {
                    m_timeToNextAttack += m_interval;
                    m_pressRemain = ATTACK_PRESS_SECONDS;
                }

                CharacterInput input;
                if (m_pressRemain > 0.0f)
                {
                    m_pressRemain -= context.deltaTime;
                    input.SetPress(EnInputButton::ActionA);
                }
                return input;
            }


        private:
            /** 攻撃する間隔(秒) */
            float m_interval;
            /** 次の攻撃までの残り時間(秒) */
            float m_timeToNextAttack;
            /** 攻撃ボタンを押し続ける残り時間(秒) */
            float m_pressRemain;
        };
    } // namespace


    DebugScene::DebugScene()
        : m_stageField(std::make_unique<actor::StageField>())
        // , m_miro(std::make_unique<actor::Miro>())
        // , m_handgun(std::make_unique<actor::Handgun>())
        , m_zombies()
        , m_freeCamera(std::make_unique<camera::FreeCamera>(PAD_INDEX))
        , m_bgPhysicsStaticObject()
        , m_stageBoundary()
    {
        // m_miro->SetPosition(MIRO_POSITION);
    }


    DebugScene::~DebugScene()
    {
        // 弾はエンジンの資源(モデル)を持つので、シーンの終了時に片付ける
        actor::BulletManager::Get().Clear();

        // ロールで傾けたカメラの上方向を戻す。(次のシーンのカメラに残さない)
        camera::FreeCamera::RestoreUp();

        // 空はエンジンが持っているので、このシーンのものを描かなくする
        RenderingEngine::Get().ClearSkyCube();
    }


    bool DebugScene::Start()
    {
        RenderingEngine::Get().SetSkyCube(SKY_CUBE_PATH, SKY_LUMINANCE);

        // 一時的にコメントアウト(Miroを使わない)
        // m_miro->StartWrapper();
        // // カメラを自由に回しても、構えている間も、Miroがカメラの向きを向かないようにする。(その場で構えるだけ)
        // // (Miroが付けたコントローラーを、差し替える)
        // m_miro->GetControllerSlot().Attach(std::make_unique<PlayerController>(PAD_INDEX, PlayerController::EnLookMode::Never));
        //
        // m_handgun->StartWrapper();
        // // ハンドガンをMiroの手に持たせる。ボーンはMiroのStart()でモデルをロードした後に取れる
        // m_handgun->AttachToBone(m_miro->GetWeaponHandBone());
        //
        // // Miroの武器の処理を、ハンドガンに結びつける。(InGameScene::Start と同じ)
        // // fire: 構えてZRを押した時。撃てたかどうか(弾切れ・リロード中は撃てない)をMiroに返す
        // // canReload・isEmpty: リロードステートに入るかの判定。Xを押した時と、マガジンが空になった時に使う
        // // reload: リロードステートに入った時
        // actor::MiroWeaponHandler weaponHandler;
        // weaponHandler.fire = [this]() { return FireHandgun(); };
        // weaponHandler.canReload = [this]() { return m_handgun->CanReload(); };
        // weaponHandler.isEmpty = [this]() { return m_handgun->GetRemainingAmmo() == 0; };
        // weaponHandler.reload = [this]() { m_handgun->Reload(); };
        // m_miro->SetWeaponHandler(std::move(weaponHandler));

        m_freeCamera->Start();

        // ステージのフィールド。ワールド行列は、ここで確定する
        m_stageField->StartWrapper();

        // 背景の当たり判定。モデルの形をそのまま使うので、ワールド行列(Z-upの補正込み)が確定した後に作る
        ModelRender& stageModelRender = m_stageField->GetModelRender();
        m_bgPhysicsStaticObject.CreateFromModel(stageModelRender.GetModel(), stageModelRender.GetModel().GetWorldMatrix());

        // ステージの外へ出られないように、外周に見えない壁を置く
        m_stageBoundary.Build();

        // ゾンビは、床の当たり判定ができた後に置く。(足元が床に着く)
        SpawnZombies();
        return true;
    }


    void DebugScene::SpawnZombies()
    {
        // 普通のゾンビ。その場で、何秒かおきに攻撃のアニメーションを出す。順にずらして、同時には振らない
        for (size_t i = 0; i < NORMAL_ZOMBIE_COUNT; ++i)
        {
            auto zombie = std::make_unique<actor::NormalZombie>();
            zombie->SetPosition(GROUP_CENTER + NORMAL_ZOMBIE_OFFSETS[i] + Vector3(0.0f, DROP_HEIGHT, 0.0f));
            Quaternion rotation;
            rotation.SetRotationYFromDirectionXZ(NORMAL_ZOMBIE_FACING);
            zombie->SetRotation(rotation);
            zombie->StartWrapper();

            const float firstDelay = ATTACK_FIRST_DELAY_STEP_SECONDS * static_cast<float>(i + 1);
            zombie->GetControllerSlot().Attach(std::make_unique<PeriodicAttackController>(ATTACK_INTERVAL_SECONDS, firstDelay));
            m_zombies.push_back(std::move(zombie));
        }

        // 走るゾンビ。GROUP_CENTER の周りに散らして置き、GROUP_CENTER へ走らせる。(着いたら攻撃する)
        const actor::RunningZombieController::PositionProvider toGroup = [](const Vector3&, const bool) -> std::optional<Vector3> { return GROUP_CENTER; };
        for (int i = 0; i < RUNNING_ZOMBIE_COUNT; ++i)
        {
            const float jitter = (i % 2 == 0) ? RUNNING_ZOMBIE_ANGLE_JITTER_DEG : -RUNNING_ZOMBIE_ANGLE_JITTER_DEG;
            const float angle = (RUNNING_ZOMBIE_FIRST_ANGLE_DEG + 360.0f * static_cast<float>(i) / RUNNING_ZOMBIE_COUNT + jitter) * PI / 180.0f;
            const Vector3 offset(std::cos(angle) * RUNNING_ZOMBIE_DISTANCE, DROP_HEIGHT, std::sin(angle) * RUNNING_ZOMBIE_DISTANCE);

            auto zombie = std::make_unique<actor::RunningZombie>();
            zombie->SetPosition(GROUP_CENTER + offset);
            // GROUP_CENTER の方を向けておく
            Quaternion rotation;
            rotation.SetRotationYFromDirectionXZ(Vector3(-offset.x, 0.0f, -offset.z));
            zombie->SetRotation(rotation);
            zombie->StartWrapper();

            auto controller = std::make_unique<actor::RunningZombieController>(zombie->GetParameter());
            controller->SetCorePositionProvider(toGroup);
            zombie->GetControllerSlot().Attach(std::move(controller));
            m_zombies.push_back(std::move(zombie));
        }
    }


    void DebugScene::Update()
    {
        m_stageField->UpdateWrapper();
        // m_miro->UpdateWrapper();
        // m_handgun->UpdateWrapper();
        for (auto& zombie : m_zombies)
        {
            zombie->UpdateWrapper();
        }
        // 弾を進める。(ポーズ中は Update() が呼ばれないので、弾も止まる)
        actor::BulletManager::Get().Update();

        // ゾンビの集まりの中心を追う。(Miroを使わない間は、固定の点)
        m_freeCamera->SetTargetPosition(GROUP_CENTER);
        m_freeCamera->Update();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
        m_stageField->RenderWrapper(rc);
        // m_miro->RenderWrapper(rc);
        // m_handgun->RenderWrapper(rc);
        for (auto& zombie : m_zombies)
        {
            zombie->RenderWrapper(rc);
        }
        actor::BulletManager::Get().Render(rc);
    }


    bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }


    bool DebugScene::IsPausable() const
    {
        return true;
    }


    // 一時的にコメントアウト(Miroを使わない)
    // bool DebugScene::FireHandgun()
    // {
    //     // 弾切れ・リロード中は撃てない
    //     if (!m_handgun->Fire())
    //     {
    //         return false;
    //     }
    //
    //     Vector3 muzzle;
    //     Quaternion barrelRotation;
    //     if (!m_handgun->CalcMuzzle(muzzle, barrelRotation))
    //     {
    //         return true;
    //     }
    //
    //     // パラメーターは、毎回読む。(jsonのホットリロードがすぐ反映される)
    //     const actor::BulletParameter& bulletParameter = ParamHolder::Get().GetParameter<actor::BulletParameter>(EnParamID::Bullet);
    //     // 弾が自分に当たらないように、Miroの体を無視する
    //     const btCollisionObject* selfBody = m_miro->GetCharacterController()->GetRigidBody()->GetBody();
    //
    //     // 壁際で銃口が壁の向こう側に出ていたら、弾を出さず、その場で着弾にする。(弾の最初の位置が壁の中だと、壁を抜けてしまう)
    //     const Vector3 chest = m_miro->GetTransform().m_position + Vector3(0.0f, CHEST_HEIGHT, 0.0f);
    //     actor::BulletHit blockedHit;
    //     if (actor::SweepSphere(chest, muzzle, bulletParameter.hitRadius, selfBody, blockedHit))
    //     {
    //         actor::BulletManager::Get().NotifyHit(blockedHit);
    //         return true;
    //     }
    //
    //     // 銃の向きそのままに撃つ。(InGameScene と違って、カメラの視線が指す点へは寄せない)
    //     actor::BulletManager::Get().Shot(barrelRotation, muzzle, bulletParameter.shotSpeed, selfBody);
    //     // 銃口の発砲炎。弾と同じ向きに出す。(エフェクトの+Zが向き)
    //     EffectManager::Get().PlayEffect(EnEffectKind::MuzzleFlash, muzzle, barrelRotation, Vector3::One);
    //     return true;
    // }
} // namespace app
