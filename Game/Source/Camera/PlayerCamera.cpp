/**
 * @file PlayerCamera.cpp
 * @brief プレイヤーを追うカメラ(TPSの肩越しの視点)
 */
#include "stdafx.h"

#include "PlayerCamera.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "AimAssistParameter.h"
#include "PlayerCameraParameter.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Util/Curve.h"
#include "Source/Option/OptionSettings.h"


namespace app
{
    namespace camera
    {
        namespace
        {
            /** 右スティックのデッドゾーン(エンジンのGamePadの INPUT_DEADZONE = 0.24 と同じ。生の値から倒し具合を求める時に引く) */
            constexpr float STICK_DEADZONE = 0.24f;
            /** XInputのスティックの値の最大(SHRT_MAX) */
            constexpr float STICK_RAW_MAX = 32767.0f;


            /**
             * @brief 右スティックを、倒し具合つきで読む
             * @details GamePad::GetRStickXF/YF は向きだけ(長さ1)なので、倒し具合は、生のXInputの値から求める。
             *          デッドゾーンの外を0〜1に直して、exponent 乗する。生の値が無い(キーボードなど)時は、倒し具合を1(全力)として扱う。
             * @param pad パッド
             * @param exponent 倒し具合の曲線の指数。0以下なら倒し具合を使わず、全力(今までと同じ)。1で比例、大きいほど浅い倒しがゆっくりになる
             * @return 右スティックの向き × 倒し具合
             */
            Vector2 ReadRightStick(GamePad& pad, const float exponent)
            {
                const float directionX = pad.GetRStickXF();
                const float directionY = pad.GetRStickYF();
                if (directionX == 0.0f && directionY == 0.0f)
                {
                    return Vector2(0.0f, 0.0f);
                }

                float amount = 1.0f;
                if (exponent > 0.0f)
                {
                    const XINPUT_GAMEPAD& raw = pad.GetXInputState().Gamepad;
                    const float rawX = static_cast<float>(raw.sThumbRX) / STICK_RAW_MAX;
                    const float rawY = static_cast<float>(raw.sThumbRY) / STICK_RAW_MAX;
                    const float length = std::sqrt(rawX * rawX + rawY * rawY);
                    if (length > FLT_EPSILON)
                    {
                        amount = std::clamp((length - STICK_DEADZONE) / (1.0f - STICK_DEADZONE), 0.0f, 1.0f);
                        amount = std::pow(amount, exponent);
                    }
                }
                return Vector2(directionX * amount, directionY * amount);
            }


            /**
             * @brief カメラの球のスイープの結果を受けるコールバック
             * @details ステージ(静的な剛体)にだけ当たる。キャラクター・ゴースト(ゾンビの攻撃判定の箱・コアの箱)は素通りする。
             */
            struct CameraSweepCallback : public btCollisionWorld::ClosestConvexResultCallback
            {
                CameraSweepCallback(const btVector3& from, const btVector3& to)
                    : ClosestConvexResultCallback(from, to)
                {}


                bool needsCollision(btBroadphaseProxy* proxy) const override
                {
                    const auto* object = static_cast<const btCollisionObject*>(proxy->m_clientObject);
                    if (!object->isStaticObject())
                    {
                        return false;
                    }
                    return ClosestConvexResultCallback::needsCollision(proxy);
                }
            };


            /**
             * @brief 始点から理想の視点へ球を動かして、壁に当たる手前までの割合を求める
             * @param from 球の始点(注視点側)
             * @param to 球の終点(理想の視点)
             * @param radius 球の半径(cm)
             * @return 当たらなければ1。当たったら、当たる手前までの割合(0〜1)
             */
            float SweepCameraFraction(const Vector3& from, const Vector3& to, const float radius)
            {
                if ((to - from).Length() < FLT_EPSILON)
                {
                    return 1.0f;
                }

                btSphereShape shape(radius);
                btTransform start;
                start.setIdentity();
                start.setOrigin(btVector3(from.x, from.y, from.z));
                btTransform end;
                end.setIdentity();
                end.setOrigin(btVector3(to.x, to.y, to.z));

                CameraSweepCallback callback(start.getOrigin(), end.getOrigin());
                PhysicsWorld::GetInstance()->ConvexSweepTest(&shape, start, end, callback);
                return callback.hasHit() ? callback.m_closestHitFraction : 1.0f;
            }
        } // namespace


        PlayerCamera::PlayerCamera(const int padIndex)
            : m_parameter(ParamHolder::Get().GetParameter<PlayerCameraParameter>(EnParamID::PlayerCamera))
            , m_padIndex(padIndex)
            , m_toCameraPos(Vector3::Zero)
            , m_targetPosition(Vector3::Zero)
            , m_isAiming(false)
            , m_aimRate(0.0f)
            , m_allowedLength(FLT_MAX)
            , m_aimAssist()
            , m_aimTargets()
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
            const float deltaTime = g_gameTime->GetFrameDeltaTime();

            // 右スティックの入力。パッド番号が範囲外、またはパッドがない場合は無入力
            // エンジンのGamePadは、スティックの値を長さ1に正規化するので、通常は入力があれば常に全力で回る。
            // 構えている間だけ、倒し具合を使う。(狙いの微調整のため)
            Vector2 stick(0.0f, 0.0f);
            if (m_padIndex >= 0 && m_padIndex < static_cast<int>(g_pad.size()) && g_pad[m_padIndex] != nullptr)
            {
                stick = ReadRightStick(*g_pad[m_padIndex], m_isAiming ? m_parameter.aimStickExponent : 0.0f);
            }
            const bool isStickMoved = stick.x != 0.0f || stick.y != 0.0f;

            // エイムアシスト。構えている間だけ。
            // 減速: 視線が狙う点に近いほど、回転速度の倍率が小さくなる。
            // 引き寄せ: スティックを倒している間だけ、狙う点までの角度の一部を、スティックの回転に足す。(何も倒していなければ、勝手には動かさない)
            float assistScale = 1.0f;
            float pullYawDeg = 0.0f;
            float pullPitchDeg = 0.0f;
            if (m_isAiming)
            {
                const AimAssistResult assist = m_aimAssist.Evaluate(g_camera3D->GetPosition(), g_camera3D->GetForward(), m_aimTargets);
                assistScale = assist.slowScale;
                if (assist.hasTarget && isStickMoved)
                {
                    const AimAssistParameter& assistParameter = ParamHolder::Get().GetParameter<AimAssistParameter>(EnParamID::AimAssist);
                    // 1フレームで詰める割合。1を超えると狙う点を行き過ぎるので、1で止める
                    const float fraction = (std::min)(assist.weight * assistParameter.pullRate * deltaTime, 1.0f);
                    pullYawDeg = assist.yawErrorDeg * fraction;
                    pullPitchDeg = assist.pitchErrorDeg * fraction;
                }
            }

            // 通常はカメラ感度、構えている間はエイム感度。どちらも、基準の回転速度に掛ける倍率。
            const OptionSettings& settings = OptionSettings::Get();
            const float sensitivity = m_isAiming ? settings.GetAimSensitivity() : settings.GetCameraSensitivity();
            const float rotateDeg = m_parameter.rotateSpeed * sensitivity * assistScale * deltaTime;

            // Y軸周りの回転(左右)。スティックを右に倒すと、右を向く
            Quaternion qRot;
            qRot.SetRotationDeg(Vector3::AxisY, rotateDeg * stick.x + pullYawDeg);
            qRot.Apply(m_toCameraPos);

            // X軸周りの回転(上下)。スティックを上に倒すと、上を向く
            // 制限を超えるときは、上下だけを取り消す。(左右の回転は残す)
            const Vector3 toCameraPosOld = m_toCameraPos;
            Vector3 axisX;
            axisX.Cross(Vector3::AxisY, m_toCameraPos);
            axisX.Normalize();
            qRot.SetRotationDeg(axisX, rotateDeg * stick.y + pullPitchDeg);
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
            const Vector3 toCamera = m_toCameraPos * distanceScale;
            const float fullLength = toCamera.Length();

            // 壁に当たる手前までの距離。始点は、横のずらしを含まない点(ターゲットの真上)にする。
            // 肩の位置が壁の外に出てしまうと、球が最初から壁に重なって当たらなくなるため。
            Vector3 pivot = m_targetPosition;
            pivot.y += m_parameter.targetOffsetUp;
            const float fraction = SweepCameraFraction(pivot, target + toCamera, m_parameter.collisionRadius);
            const float hitLength = (std::max)(fullLength * fraction, (std::min)(m_parameter.collisionMinDistance, fullLength));

            // 壁に近づく時は即座に縮め、離れた時だけ、ゆっくり元の距離へ戻す。
            if (hitLength < m_allowedLength)
            {
                m_allowedLength = hitLength;
            }
            else
            {
                m_allowedLength = (std::min)(m_allowedLength + m_parameter.collisionRecoverSpeed * deltaTime, hitLength);
            }
            const float usedLength = (std::min)(fullLength, m_allowedLength);
            const Vector3 pos = fullLength > FLT_EPSILON ? target + toCamera * (usedLength / fullLength) : target;

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
