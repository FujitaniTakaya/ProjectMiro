/**
 * @file ZombieCoreApproach.cpp
 * @brief ゾンビがコアへ向かう処理
 */
#include "stdafx.h"

#include "ZombieCoreApproach.h"

#include "ZombieParameter.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 方向ベクトルを正規化してよい長さの二乗のしきい値(ゼロ除算防止。AIControllerと同じ値) */
            constexpr float DIRECTION_NORMALIZE_SQ = 0.001f;
        } // namespace


        ZombieCoreApproach::ZombieCoreApproach(const ZombieParameter& parameter)
            : m_parameter(parameter)
            , m_corePositionProvider(nullptr)
        {
        }


        void ZombieCoreApproach::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_corePositionProvider = provider;
        }


        bool ZombieCoreApproach::CalcToCore(const ControllerContext& context, Vector3& outToCore, bool& outIsArrived) const
        {
            // コアがなければ求められない
            Vector3 corePosition(Vector3::Zero);
            if (!m_corePositionProvider || !m_corePositionProvider(corePosition))
            {
                return false;
            }

            // 高さは無視して、XZ平面で測る
            outToCore = Vector3(corePosition.x - context.position.x, 0.0f, corePosition.z - context.position.z);
            const float distanceSq = outToCore.LengthSq();

            // コアに着いたとみなす。真上・真下にいて向きが決まらないときも、着いた扱いにする
            const float stopDistance = m_parameter.coreStopDistance;
            outIsArrived = distanceSq <= stopDistance * stopDistance || distanceSq <= DIRECTION_NORMALIZE_SQ;
            return true;
        }


        bool ZombieCoreApproach::Approach(const ControllerContext& context, CharacterInput& input) const
        {
            // コアがなければ、コアに着いていれば向かわない
            Vector3 toCore(Vector3::Zero);
            bool isArrived = false;
            if (!CalcToCore(context, toCore, isArrived) || isArrived)
            {
                return false;
            }

            toCore.Normalize();
            input.SetMove(toCore);
            return true;
        }


        bool ZombieCoreApproach::IsArrived(const ControllerContext& context) const
        {
            Vector3 toCore(Vector3::Zero);
            bool isArrived = false;
            return CalcToCore(context, toCore, isArrived) && isArrived;
        }
    } // namespace actor
} // namespace app
