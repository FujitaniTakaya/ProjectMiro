/**
 * @file AimAssist.cpp
 * @brief 構え中の狙いやすさを助ける判定(エイムアシスト)
 */
#include "stdafx.h"

#include "AimAssist.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "AimAssistParameter.h"
#include "Source/Core/Param/ParamHolder.h"


namespace app
{
    namespace camera
    {
        AimAssistResult::AimAssistResult()
            : hasTarget(false)
            , weight(0.0f)
            , slowScale(1.0f)
            , yawErrorDeg(0.0f)
            , pitchErrorDeg(0.0f)
        {
        }


        AimAssist::AimAssist()
            : m_parameter(ParamHolder::Get().GetParameter<AimAssistParameter>(EnParamID::AimAssist))
        {
        }


        AimAssist::~AimAssist() = default;


        AimAssistResult AimAssist::Evaluate(const Vector3& cameraPosition, const Vector3& cameraForward, const std::vector<Vector3>& targets) const
        {
            AimAssistResult result;

            Vector3 forward = cameraForward;
            forward.Normalize();

            // 水平の前と右。PlayerCamera と同じ求め方。真上・真下を向くと水平成分が無く、左右の向きが決まらない
            Vector3 forwardH(forward.x, 0.0f, forward.z);
            if (forwardH.LengthSq() < FLT_EPSILON)
            {
                return result;
            }
            forwardH.Normalize();
            const Vector3 rightH(forwardH.z, 0.0f, -forwardH.x);

            // 対象の中から、視線に垂直な距離が最も小さい点を選ぶ。assistRadius 以上のものは候補にしない
            float bestPerpendicular = m_parameter.assistRadius;
            Vector3 bestToTarget = Vector3::Zero;
            for (const Vector3& target : targets)
            {
                const Vector3 toTarget = target - cameraPosition;
                // 視線方向の奥行き。カメラの後ろは対象にしない
                const float depth = Dot(toTarget, forward);
                if (depth <= 0.0f || toTarget.Length() > m_parameter.maxDistance)
                {
                    continue;
                }
                // 視線に垂直な成分の長さ = 視線から点までの距離
                const float perpendicular = (toTarget - forward * depth).Length();
                if (perpendicular < bestPerpendicular)
                {
                    bestPerpendicular = perpendicular;
                    bestToTarget = toTarget;
                    result.hasTarget = true;
                }
            }
            if (!result.hasTarget)
            {
                return result;
            }

            // 近さ。ど真ん中で1、assistRadius で0。縁で急に効き始めないように、滑らかに変える(smoothstep)
            const float t = 1.0f - bestPerpendicular / m_parameter.assistRadius;
            result.weight = t * t * (3.0f - 2.0f * t);
            result.slowScale = 1.0f + (m_parameter.slowScale - 1.0f) * result.weight;

            // 角度の誤差。左右は水平面での向きの差、上下は水平からの仰角の差
            const float forwardLength = Dot(bestToTarget, forwardH);
            const float rightLength = Dot(bestToTarget, rightH);
            const float horizontalLength = std::sqrt(forwardLength * forwardLength + rightLength * rightLength);
            const float yawToTarget = std::atan2(rightLength, forwardLength);
            const float pitchToTarget = std::atan2(bestToTarget.y, horizontalLength);
            const float pitchOfView = std::asin(std::clamp(forward.y, -1.0f, 1.0f));
            result.yawErrorDeg = Math::RadToDeg(yawToTarget);
            result.pitchErrorDeg = Math::RadToDeg(pitchToTarget - pitchOfView);
            return result;
        }
    } // namespace camera
} // namespace app
