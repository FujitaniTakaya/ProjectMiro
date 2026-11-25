/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <memory>
#include <utility>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Actor/Stage/StageField.h"
#include "Source/Actor/Weapon/Bullet/BulletManager.h"
#include "Source/Actor/Weapon/Bullet/BulletParameter.h"
#include "Source/Actor/Weapon/Bullet/BulletSweep.h"
#include "Source/Actor/Weapon/Handgun/Handgun.h"
#include "Source/Camera/FreeCamera.h"
#include "Source/Controller/PlayerController.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
#include "Source/Effect/EffectManager.h"
#include "Source/MoveBehavior/ControllerSlot.h"


namespace app
{
    namespace
    {
        /** 空のキューブマップ(曇天の昼)。InGameScene と同じ */
        constexpr const wchar_t* SKY_CUBE_PATH = L"Assets/modelData/preset/skyCubeMapOvercast.dds";
        /** 空の明るさ(キューブマップの色に掛ける倍率) */
        constexpr float SKY_LUMINANCE = 1.0f;
        /** Miroの初期座標。InGameScene と同じ */
        const Vector3 MIRO_POSITION(100.0f, 100.0f, 0.0f);
        /** Miroの足元から胸までの高さ(cm)。壁に銃口が出ていないかを、胸から銃口までの球で調べる時の始点(Miroの全高は160)。InGameScene と同じ */
        constexpr float CHEST_HEIGHT = 120.0f;
        /** 入力を読み取るパッドの番号 */
        constexpr int PAD_INDEX = 0;
    } // namespace


    DebugScene::DebugScene()
        : m_stageField(std::make_unique<actor::StageField>())
        , m_miro(std::make_unique<actor::Miro>())
        , m_handgun(std::make_unique<actor::Handgun>())
        , m_freeCamera(std::make_unique<camera::FreeCamera>(PAD_INDEX))
        , m_bgPhysicsStaticObject()
        , m_stageBoundary()
    {
        m_miro->SetPosition(MIRO_POSITION);
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

        m_miro->StartWrapper();
        // カメラを自由に回しても、構えている間も、Miroがカメラの向きを向かないようにする。(その場で構えるだけ)
        // (Miroが付けたコントローラーを、差し替える)
        m_miro->GetControllerSlot().Attach(std::make_unique<PlayerController>(PAD_INDEX, PlayerController::EnLookMode::Never));

        m_handgun->StartWrapper();
        // ハンドガンをMiroの手に持たせる。ボーンはMiroのStart()でモデルをロードした後に取れる
        m_handgun->AttachToBone(m_miro->GetWeaponHandBone());

        // Miroの武器の処理を、ハンドガンに結びつける。(InGameScene::Start と同じ)
        // fire: 構えてZRを押した時。撃てたかどうか(弾切れ・リロード中は撃てない)をMiroに返す
        // canReload・isEmpty: リロードステートに入るかの判定。Xを押した時と、マガジンが空になった時に使う
        // reload: リロードステートに入った時
        actor::MiroWeaponHandler weaponHandler;
        weaponHandler.fire = [this]() { return FireHandgun(); };
        weaponHandler.canReload = [this]() { return m_handgun->CanReload(); };
        weaponHandler.isEmpty = [this]() { return m_handgun->GetRemainingAmmo() == 0; };
        weaponHandler.reload = [this]() { m_handgun->Reload(); };
        m_miro->SetWeaponHandler(std::move(weaponHandler));

        m_freeCamera->Start();

        // ステージのフィールド。ワールド行列は、ここで確定する
        m_stageField->StartWrapper();

        // 背景の当たり判定。モデルの形をそのまま使うので、ワールド行列(Z-upの補正込み)が確定した後に作る
        ModelRender& stageModelRender = m_stageField->GetModelRender();
        m_bgPhysicsStaticObject.CreateFromModel(stageModelRender.GetModel(), stageModelRender.GetModel().GetWorldMatrix());

        // ステージの外へ出られないように、外周に見えない壁を置く
        m_stageBoundary.Build();
        return true;
    }


    void DebugScene::Update()
    {
        m_stageField->UpdateWrapper();
        m_miro->UpdateWrapper();
        m_handgun->UpdateWrapper();
        // 弾を進める。(ポーズ中は Update() が呼ばれないので、弾も止まる)
        actor::BulletManager::Get().Update();

        // Miroが動いた後の座標を追う
        m_freeCamera->SetTargetPosition(m_miro->GetTransform().m_position);
        m_freeCamera->Update();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
        m_stageField->RenderWrapper(rc);
        m_miro->RenderWrapper(rc);
        m_handgun->RenderWrapper(rc);
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


    bool DebugScene::FireHandgun()
    {
        // 弾切れ・リロード中は撃てない
        if (!m_handgun->Fire())
        {
            return false;
        }

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

        // 壁際で銃口が壁の向こう側に出ていたら、弾を出さず、その場で着弾にする。(弾の最初の位置が壁の中だと、壁を抜けてしまう)
        const Vector3 chest = m_miro->GetTransform().m_position + Vector3(0.0f, CHEST_HEIGHT, 0.0f);
        actor::BulletHit blockedHit;
        if (actor::SweepSphere(chest, muzzle, bulletParameter.hitRadius, selfBody, blockedHit))
        {
            actor::BulletManager::Get().NotifyHit(blockedHit);
            return true;
        }

        // 銃の向きそのままに撃つ。(InGameScene と違って、カメラの視線が指す点へは寄せない)
        actor::BulletManager::Get().Shot(barrelRotation, muzzle, bulletParameter.shotSpeed, selfBody);
        // 銃口の発砲炎。弾と同じ向きに出す。(エフェクトの+Zが向き)
        EffectManager::Get().PlayEffect(EnEffectKind::MuzzleFlash, muzzle, barrelRotation, Vector3::One);
        return true;
    }
} // namespace app
