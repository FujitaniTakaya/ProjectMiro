/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <memory>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Actor/Stage/StageField.h"
#include "Source/Actor/Weapon/Handgun/Handgun.h"
#include "Source/Camera/PlayerCamera.h"
#include "Source/Manager/InGameInfoHub.h"
#include "Source/UI/Handgun/HandgunAmmoMenu.h"
#include "Source/UI/PlayerHP/HPBarMenu.h"


namespace app
{
    namespace
    {
        /** プレイヤーのHPバーのレイアウトのjson */
        constexpr const char* HP_BAR_JSON_PATH = "Assets/parameter/ui/HPBar.json";
        /** ハンドガンの残段数UIのレイアウトのjson */
        constexpr const char* HANDGUN_AMMO_JSON_PATH = "Assets/parameter/ui/HandgunAmmo.json";
        /** レティクルのレイアウトのjson */
        constexpr const char* RETICLE_JSON_PATH = "Assets/parameter/ui/Reticle.json";
        /** ハンドガンを置く座標。手に持たせるまでは、カメラの注視点(Miroの右上)の手前に置く */
        const Vector3 HANDGUN_POSITION(60.0f, 90.0f, -60.0f);
        /** ハンドガンのY軸回転(度)。銃口が+Xを向き、側面が最初のカメラ(Miroの後ろ)から見える */
        constexpr float HANDGUN_YAW_DEG = 90.0f;
    } // namespace


    DebugScene::DebugScene()
        : m_miro(nullptr)
        , m_playerCamera(nullptr)
        , m_hpBarPacket(nullptr)
        , m_handgun(nullptr)
        , m_ammoPacket(nullptr)
        , m_reticlePacket(nullptr)
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

        m_stageField = std::make_unique<actor::StageField>();
        m_miro = std::make_unique<actor::Miro>();
        m_handgun = std::make_unique<actor::Handgun>();
        m_handgun->SetPosition(HANDGUN_POSITION);
        Quaternion handgunRotation;
        handgunRotation.SetRotationDegY(HANDGUN_YAW_DEG);
        m_handgun->SetRotation(handgunRotation);
        m_playerCamera = std::make_unique<camera::PlayerCamera>();
    }


    DebugScene::~DebugScene()
    {
        // 登録した関数は、HPバーを参照している。HPバーが無くなる前に解除する
        InGameInfoHub::Get().ResetObservers();
    }


    bool DebugScene::Start()
    {
        m_miro->StartWrapper();
        m_handgun->StartWrapper();
        m_playerCamera->Start();

        // ステージのフィールド。ワールド行列は、ここで確定する
        m_stageField->StartWrapper();

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
        m_miro->UpdateWrapper();
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

        // Miroが動いた後の座標を追う
        m_playerCamera->SetTargetPosition(m_miro->GetTransform().m_position);
        m_playerCamera->Update();

        // MiroがHPを通知した後に、HPバーを更新する
        m_hpBarPacket->Update();

        // ハンドガンが残弾数を通知した後に、残段数UIを更新する
        m_ammoPacket->Update();
        m_reticlePacket->Update();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
        m_stageField->RenderWrapper(rc);
        m_miro->RenderWrapper(rc);
        m_handgun->RenderWrapper(rc);
    }


    void DebugScene::RenderUI(RenderContext& rc)
    {
        m_hpBarPacket->Render(rc);
        m_ammoPacket->Render(rc);
        m_reticlePacket->Render(rc);
    }


    bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }
} // namespace app
