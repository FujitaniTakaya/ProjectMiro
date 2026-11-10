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


        bool ZombieCoreApproach::Approach(const ControllerContext& context, CharacterInput& input) const
        {
            // コアがなければ向かわない
            Vector3 corePosition(Vector3::Zero);
            if (!m_corePositionProvider || !m_corePositionProvider(corePosition))
            {
                return false;
            }

            // 高さは無視して、XZ平面でコアへ向かう
            Vector3 toCore(corePosition.x - context.position.x, 0.0f, corePosition.z - context.position.z);
            const float distanceSq = toCore.LengthSq();

            // コアに着いていれば向かわない。真上・真下にいて向きが決まらないときも、動かない
            const float stopDistance = m_parameter.coreStopDistance;
            if (distanceSq <= stopDistance * stopDistance || distanceSq <= DIRECTION_NORMALIZE_SQ)
            {
                return false;
            }

            toCore.Normalize();
            input.SetMove(toCore);
            return true;
        }
    } // namespace actor
} // namespace app
