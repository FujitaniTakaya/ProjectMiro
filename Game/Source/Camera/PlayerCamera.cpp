/**
 * @file PlayerCamera.cpp
 * @brief プレイヤーを追うカメラ(TPSの肩越しの視点)
 */
#include "stdafx.h"

#include "PlayerCamera.h"

#include <algorithm>
#include <cfloat>

#include "PlayerCameraParameter.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Util/Curve.h"
#include "Source/Option/OptionSettings.h"


namespace app
{
    namespace camera
    {
        PlayerCamera::PlayerCamera(const int padIndex)
            : m_parameter(ParamHolder::Get().GetParameter<PlayerCameraParameter>(EnParamID::PlayerCamera))
            , m_padIndex(padIndex)
            , m_toCameraPos(Vector3::Zero)
            , m_targetPosition(Vector3::Zero)
            , m_isAiming(false)
            , m_aimRate(0.0f)
        {
        }


        PlayerCamera::~PlayerCamera() = default;


        void PlayerCamera::Start()
        {
            // 注視点から視点までのベクトルを設定
            m_toCameraPos = m_parameter.toCameraPos;
        }


        void PlayerCamera::Update()
        {
            // 右スティックの入力。パッド番号が範囲外、またはパッドがない場合は無入力
            // エンジンのGamePadは、スティックの値を長さ1に正規化するので、入力があれば常に全力で回る。
            float stickX = 0.0f;
            float stickY = 0.0f;
            if (m_padIndex >= 0 && m_padIndex < static_cast<int>(g_pad.size()) && g_pad[m_padIndex] != nullptr)
            {
                stickX = g_pad[m_padIndex]->GetRStickXF();
                stickY = g_pad[m_padIndex]->GetRStickYF();
            }
            // 通常はカメラ感度、構えている間はエイム感度。どちらも、基準の回転速度に掛ける倍率。
            const OptionSettings& settings = OptionSettings::Get();
            const float sensitivity = m_isAiming ? settings.GetAimSensitivity() : settings.GetCameraSensitivity();
            const float rotateDeg = m_parameter.rotateSpeed * sensitivity * g_gameTime->GetFrameDeltaTime();

            // Y軸周りの回転(左右)
            Quaternion qRot;
            qRot.SetRotationDeg(Vector3::AxisY, rotateDeg * stickX);
            qRot.Apply(m_toCameraPos);

            // X軸周りの回転(上下)
            // 制限を超えるときは、上下だけを取り消す。(左右の回転は残す)
            const Vector3 toCameraPosOld = m_toCameraPos;
            Vector3 axisX;
            axisX.Cross(Vector3::AxisY, m_toCameraPos);
            axisX.Normalize();
            qRot.SetRotationDeg(axisX, rotateDeg * stickY);
            qRot.Apply(m_toCameraPos);

            Vector3 toPosDir = m_toCameraPos;
            toPosDir.Normalize();
            if (toPosDir.y < m_parameter.minToCameraDirY || toPosDir.y > m_parameter.maxToCameraDirY)
            {
                // 上向きすぎ、または下向きすぎ
                m_toCameraPos = toCameraPosOld;
            }

            // カメラの水平方向の前と右を、回した後の向きから求める。(視点から注視点に向かう向き)
            Vector3 cameraForward = Vector3::Front;
            Vector3 toTarget(-m_toCameraPos.x, 0.0f, -m_toCameraPos.z);
            if (toTarget.LengthSq() > FLT_EPSILON)
            {
                toTarget.Normalize();
                cameraForward = toTarget;
            }
            const Vector3 cameraRight(cameraForward.z, 0.0f, -cameraForward.x);

            // 注視点は、ターゲットの右上
            Vector3 target = m_targetPosition + cameraRight * m_parameter.targetOffsetRight;
            target.y += m_parameter.targetOffsetUp;

            // 構えの度合いを、構え中は1へ、そうでなければ0へ動かす。(切り替えの時間が0以下なら、一瞬で切り替える)
            const float aimStep = m_parameter.aimBlendSeconds > FLT_EPSILON ? g_gameTime->GetFrameDeltaTime() / m_parameter.aimBlendSeconds : 1.0f;
            m_aimRate = m_isAiming ? (std::min)(m_aimRate + aimStep, 1.0f) : (std::max)(m_aimRate - aimStep, 0.0f);
            const float aimEased = util::ApplyEasing(util::EasingType::EaseInOut, m_aimRate);

            // 視点を計算する。構え中は、回転後のベクトルは触らずに、距離だけ縮めたものを使う。
            const float distanceScale = 1.0f + (m_parameter.aimDistanceScale - 1.0f) * aimEased;
            const Vector3 pos = target + m_toCameraPos * distanceScale;

            // メインカメラに設定して更新する。
            g_camera3D->SetNear(m_parameter.nearClip);
            g_camera3D->SetFar(m_parameter.farClip);
            g_camera3D->SetTarget(target);
            g_camera3D->SetPosition(pos);
            g_camera3D->Update();
        }


        void PlayerCamera::SetTargetPosition(const Vector3& position)
        {
            m_targetPosition = position;
        }
    } // namespace camera
} // namespace app
