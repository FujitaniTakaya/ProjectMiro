/**
 * @file FreeCamera.cpp
 * @brief ターゲットの周りを自由に動くカメラ(撮影・デバッグ用)
 */
#include "stdafx.h"

#include "FreeCamera.h"

#include <algorithm>
#include <cmath>


namespace app
{
    namespace camera
    {
        namespace
        {
            /** 注視点の高さ(足元から。cm)。Miroの全高は160 */
            constexpr float TARGET_HEIGHT = 100.0f;
            /** 周回の速さ(度/秒)。右スティックを倒し切った時 */
            constexpr float ROTATE_SPEED_DEG = 120.0f;
            /** ロールの速さ(度/秒) */
            constexpr float ROLL_SPEED_DEG = 45.0f;
            /** ズームの速さ。1秒押すと、距離が、この数だけ2倍(または1/2)になる */
            constexpr float ZOOM_SPEED_OCTAVES = 1.0f;
            /** 距離の最小(cm) */
            constexpr float MIN_DISTANCE = 40.0f;
            /** 距離の最大(cm) */
            constexpr float MAX_DISTANCE = 1500.0f;
            /** 初期状態の「注視点→視点」。PlayerCamera の初期値と同じ */
            const Vector3 DEFAULT_TO_CAMERA_POS(0.0f, 50.0f, -250.0f);
            /** 上下の角度の限界(度)。真上・真下では、左右の向きが決まらなくなる */
            constexpr float PITCH_LIMIT_DEG = 85.0f;
            /** ロールの角度の範囲(度)。±この値で折り返す */
            constexpr float ROLL_WRAP_DEG = 180.0f;
            /** 近クリップ距離(cm) */
            constexpr float NEAR_CLIP = 1.0f;
            /** 遠クリップ距離(cm) */
            constexpr float FAR_CLIP = 10000.0f;


            /**
             * @brief 角度を、±ROLL_WRAP_DEG の範囲に折り返す
             * @param angleDeg 角度(度)
             * @return 折り返した角度(度)
             */
            float WrapRoll(const float angleDeg)
            {
                float wrapped = std::fmod(angleDeg + ROLL_WRAP_DEG, ROLL_WRAP_DEG * 2.0f);
                if (wrapped < 0.0f)
                {
                    wrapped += ROLL_WRAP_DEG * 2.0f;
                }
                return wrapped - ROLL_WRAP_DEG;
            }
        } // namespace


        FreeCamera::FreeCamera(const int padIndex)
            : m_padIndex(padIndex)
            , m_targetPosition(Vector3::Zero)
            , m_toCameraDir(Vector3::Zero)
            , m_distance(0.0f)
            , m_rollDeg(0.0f)
        {
        }


        FreeCamera::~FreeCamera() = default;


        void FreeCamera::Start()
        {
            m_distance = DEFAULT_TO_CAMERA_POS.Length();
            m_toCameraDir = DEFAULT_TO_CAMERA_POS;
            m_toCameraDir.Normalize();
            m_rollDeg = 0.0f;
        }


        void FreeCamera::Update()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();

            // パッド番号が範囲外、またはパッドがない場合は、操作しない(今の位置のまま)
            if (m_padIndex >= 0 && m_padIndex < static_cast<int>(g_pad.size()) && g_pad[m_padIndex] != nullptr)
            {
                GamePad& pad = *g_pad[m_padIndex];

                // 周回。PlayerCamera と同じ規則(右スティックを右に倒すと右を向く、上に倒すと上を向く)
                const float rotateDeg = ROTATE_SPEED_DEG * deltaTime;
                Quaternion rotation;
                rotation.SetRotationDeg(Vector3::AxisY, rotateDeg * pad.GetRStickXF());
                rotation.Apply(m_toCameraDir);

                // 上下。制限を超える時は、上下だけを取り消す。(左右の回転は残す)
                const Vector3 toCameraDirOld = m_toCameraDir;
                Vector3 axisX;
                axisX.Cross(Vector3::AxisY, m_toCameraDir);
                axisX.Normalize();
                rotation.SetRotationDeg(axisX, rotateDeg * pad.GetRStickYF());
                rotation.Apply(m_toCameraDir);
                m_toCameraDir.Normalize();
                if (std::fabs(m_toCameraDir.y) > std::sin(Math::DegToRad(PITCH_LIMIT_DEG)))
                {
                    m_toCameraDir = toCameraDirOld;
                }

                // ズーム。RBで近づき、LBで離れる。距離の倍率で変えるので、近いほど細かく、遠いほど速く動く
                const float zoom = (pad.IsPress(enButtonLB1) ? 1.0f : 0.0f) - (pad.IsPress(enButtonRB1) ? 1.0f : 0.0f);
                m_distance = std::clamp(m_distance * std::exp2(zoom * ZOOM_SPEED_OCTAVES * deltaTime), MIN_DISTANCE, MAX_DISTANCE);

                // ロール。十字キーの右で時計回り
                const float roll = (pad.IsPress(enButtonRight) ? 1.0f : 0.0f) - (pad.IsPress(enButtonLeft) ? 1.0f : 0.0f);
                m_rollDeg = WrapRoll(m_rollDeg + roll * ROLL_SPEED_DEG * deltaTime);

                // 初期状態へ戻す
                if (pad.IsTrigger(enButtonSelect))
                {
                    Start();
                }
            }

            const Vector3 target = m_targetPosition + Vector3(0.0f, TARGET_HEIGHT, 0.0f);
            const Vector3 position = target + m_toCameraDir * m_distance;

            // ロール。上方向(Y軸)を、視線の周りに回す
            Vector3 forward = target - position;
            forward.Normalize();
            Vector3 up = Vector3::AxisY;
            Quaternion rollRotation;
            rollRotation.SetRotationDeg(forward, m_rollDeg);
            rollRotation.Apply(up);

            g_camera3D->SetNear(NEAR_CLIP);
            g_camera3D->SetFar(FAR_CLIP);
            g_camera3D->SetUp(up);
            g_camera3D->SetTarget(target);
            g_camera3D->SetPosition(position);
            g_camera3D->Update();
        }


        void FreeCamera::RestoreUp()
        {
            if (g_camera3D != nullptr)
            {
                g_camera3D->SetUp(Vector3::AxisY);
            }
        }
    } // namespace camera
} // namespace app
