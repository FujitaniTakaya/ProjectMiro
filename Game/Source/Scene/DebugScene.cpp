/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <algorithm>
#include <cmath>

#include "Source/Actor/Weapon/Bullet/BulletManager.h"
#include "Source/Actor/Weapon/Handgun/Handgun.h"


namespace app
{
    namespace
    {
        /** 空のキューブマップ(曇天の昼)。InGameScene と同じ */
        constexpr const wchar_t* SKY_CUBE_PATH = L"Assets/modelData/preset/skyCubeMapOvercast.dds";
        /** 空の明るさ(キューブマップの色に掛ける倍率) */
        constexpr float SKY_LUMINANCE = 1.0f;
        /** ハンドガンを置く座標(Y-up。床は無い) */
        const Vector3 HANDGUN_POSITION(0.0f, 150.0f, 0.0f);
        /** 弾の発射スピードの初期値(cm/秒)。発射位置と向きを目で確認できるよう、実際のゲームより遅くしてある */
        constexpr float DEFAULT_BULLET_SPEED = 300.0f;
        /** ImGuiのスライダーで選べる弾のスピード(cm/秒) */
        constexpr float MIN_BULLET_SPEED = 10.0f;
        constexpr float MAX_BULLET_SPEED = 10000.0f;
        /** カメラの左右の角度の初期値(度) */
        constexpr float DEFAULT_CAMERA_YAW_DEG = 30.0f;
        /** カメラの上下の角度の初期値(度) */
        constexpr float DEFAULT_CAMERA_PITCH_DEG = 15.0f;
        /** カメラからハンドガンまでの距離の初期値(cm) */
        constexpr float DEFAULT_CAMERA_DISTANCE = 80.0f;
        /** 右スティックを倒し切った時の、カメラの回る速さ(度/秒) */
        constexpr float CAMERA_ROTATE_SPEED_DEG = 90.0f;
        /** カメラの上下の角度の限界(度)。真上・真下では、左右の向きが決まらなくなる */
        constexpr float CAMERA_PITCH_LIMIT_DEG = 80.0f;
        /** カメラの近クリップ距離(cm)。ハンドガンに寄れるように、小さくする */
        constexpr float CAMERA_NEAR = 1.0f;
        /** カメラの遠クリップ距離(cm)。飛んでいく弾が見えなくなるまで */
        constexpr float CAMERA_FAR = 20000.0f;
        /** パッドの番号 */
        constexpr int PAD_INDEX = 0;
    } // namespace


    DebugScene::DebugScene()
        : m_handgun(std::make_unique<actor::Handgun>())
        , m_bulletSpeed(DEFAULT_BULLET_SPEED)
        , m_cameraYawDeg(DEFAULT_CAMERA_YAW_DEG)
        , m_cameraPitchDeg(DEFAULT_CAMERA_PITCH_DEG)
        , m_cameraDistance(DEFAULT_CAMERA_DISTANCE)
    {}


    DebugScene::~DebugScene()
    {
        // 弾はエンジンの資源(モデル)を持つので、シーンの終了時に片付ける
        actor::BulletManager::Get().Clear();
        // 空はエンジンが持っているので、このシーンのものを描かなくする
        RenderingEngine::Get().ClearSkyCube();
    }


    bool DebugScene::Start()
    {
        RenderingEngine::Get().SetSkyCube(SKY_CUBE_PATH, SKY_LUMINANCE);

        m_handgun->SetPosition(HANDGUN_POSITION);
        m_handgun->StartWrapper();

        UpdateCamera();
        return true;
    }


    void DebugScene::Update()
    {
        // 取り付けは無いので、置いた座標のまま。モデルとボーンのワールド行列、アニメーションが更新される
        m_handgun->UpdateWrapper();

        // 操作: ZR=撃つ、X=リロード、右スティック=カメラを回す
        if (PAD_INDEX < static_cast<int>(g_pad.size()) && g_pad[PAD_INDEX] != nullptr)
        {
            GamePad& pad = *g_pad[PAD_INDEX];
            if (pad.IsTrigger(enButtonRB2))
            {
                Fire();
            }
            if (pad.IsTrigger(enButtonX))
            {
                m_handgun->Reload();
            }
            const float rotate = CAMERA_ROTATE_SPEED_DEG * g_gameTime->GetFrameDeltaTime();
            m_cameraYawDeg += pad.GetRStickXF() * rotate;
            m_cameraPitchDeg += pad.GetRStickYF() * rotate;
        }

        actor::BulletManager::Get().Update();

#ifdef BALLOON_IMGUI_ENABLED
        if (ImGui::Begin("Handgun / Bullet (DEBUG)"))
        {
            ImGui::Text(
                "ammo: %u / %u%s  bullets: %zu",
                m_handgun->GetRemainingAmmo(),
                m_handgun->GetMagazineSize(),
                m_handgun->IsReloading() ? " (reloading)" : "",
                actor::BulletManager::Get().GetBulletCount()
            );
            if (ImGui::Button("Fire"))
            {
                Fire();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload"))
            {
                m_handgun->Reload();
            }
            ImGui::SliderFloat("bullet speed", &m_bulletSpeed, MIN_BULLET_SPEED, MAX_BULLET_SPEED);
            ImGui::SliderFloat("camera yaw", &m_cameraYawDeg, -180.0f, 180.0f);
            ImGui::SliderFloat("camera pitch", &m_cameraPitchDeg, -CAMERA_PITCH_LIMIT_DEG, CAMERA_PITCH_LIMIT_DEG);
            ImGui::SliderFloat("camera distance", &m_cameraDistance, 20.0f, 400.0f);
        }
        ImGui::End();
#endif // BALLOON_IMGUI_ENABLED

        UpdateCamera();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
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


    void DebugScene::Fire()
    {
        // 弾切れ・リロード中は撃てない
        if (!m_handgun->Fire())
        {
            return;
        }

        Vector3 position;
        Quaternion rotation;
        if (m_handgun->CalcMuzzle(position, rotation))
        {
            actor::BulletManager::Get().Shot(rotation, position, m_bulletSpeed);
        }
    }


    void DebugScene::UpdateCamera()
    {
        m_cameraPitchDeg = std::clamp(m_cameraPitchDeg, -CAMERA_PITCH_LIMIT_DEG, CAMERA_PITCH_LIMIT_DEG);

        // ハンドガンを注視して周回する(Y-up)
        const float yaw = Math::DegToRad(m_cameraYawDeg);
        const float pitch = Math::DegToRad(m_cameraPitchDeg);
        const Vector3 toCamera(std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch));

        g_camera3D->SetNear(CAMERA_NEAR);
        g_camera3D->SetFar(CAMERA_FAR);
        g_camera3D->SetTarget(HANDGUN_POSITION);
        g_camera3D->SetPosition(HANDGUN_POSITION + toCamera * m_cameraDistance);
        g_camera3D->Update();
    }
} // namespace app
