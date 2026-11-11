/**
 * @file ZombieCoreApproach.cpp
 * @brief ゾンビがコアへ向かう処理
 */
#include "stdafx.h"

#include "ZombieCoreApproach.h"

#include <algorithm>
#include <vector>

#include "Source/Actor/Character/Zombie/ZombieParameter.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
#include "Source/Navigation/NavigationParameter.h"
#include "Source/Navigation/NavigationService.h"


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
            , m_navParameter(ParamHolder::Get().GetParameter<nav::NavigationParameter>(EnParamID::Navigation))
            , m_corePositionProvider(nullptr)
            , m_pathFollower()
            , m_repathCooldown(0.0f)
        {
        }


        void ZombieCoreApproach::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_corePositionProvider = provider;
        }


        std::optional<ZombieCoreApproach::CoreApproachInfo> ZombieCoreApproach::CalcToCore(const ControllerContext& context) const
        {
            // コアがなければ求められない
            if (!m_corePositionProvider)
            {
                return std::nullopt;
            }
            const std::optional<Vector3> corePosition = m_corePositionProvider();
            if (!corePosition)
            {
                return std::nullopt;
            }

            // 高さは無視して、XZ平面で測る
            const Vector3 toCore(corePosition->x - context.position.x, 0.0f, corePosition->z - context.position.z);
            const float distanceSq = toCore.LengthSq();

            // コアに着いたとみなす。真上・真下にいて向きが決まらないときも、着いた扱いにする
            const float stopDistance = m_parameter.coreStopDistance;
            const bool isArrived = distanceSq <= stopDistance * stopDistance || distanceSq <= DIRECTION_NORMALIZE_SQ;
            return CoreApproachInfo{ *corePosition, toCore, isArrived };
        }


        std::optional<Vector3> ZombieCoreApproach::CalcPathDirection(const ControllerContext& context, const Vector3& corePosition)
        {
            nav::NavigationService& navigation = nav::NavigationService::Get();
            if (!navigation.IsReady())
            {
                m_pathFollower.Clear();
                return std::nullopt;
            }

            // 経路をたどる状況を進める
            m_repathCooldown = std::max<float>(m_repathCooldown - context.deltaTime, 0.0f);
            nav::PathFollowSettings followSettings;
            followSettings.reachRadius = m_navParameter.reachRadius;
            followSettings.maxDeviation = m_navParameter.maxDeviation;
            followSettings.stallTime = m_navParameter.stallTime;
            followSettings.stallDistance = m_navParameter.stallDistance;
            const nav::EnFollowStatus status = m_pathFollower.Update(context.position.x, context.position.z, context.deltaTime, followSettings);

            // 経路が無い・終わった・外れた・進めなくなったら、探し直す。失敗しても、次に探すのは間隔を空ける
            if (status != nav::EnFollowStatus::Following && m_repathCooldown <= 0.0f)
            {
                m_repathCooldown = m_navParameter.repathInterval;
                const std::optional<std::vector<nav::NavVec3>> path = navigation.FindPath(context.position, corePosition);
                if (path)
                {
                    m_pathFollower.SetPath(*path);
                }
                else
                {
                    m_pathFollower.Clear();
                }
            }

            const std::optional<nav::NavVec2> direction = m_pathFollower.GetMoveDirection(context.position.x, context.position.z);
            if (!direction)
            {
                return std::nullopt;
            }
            return Vector3(direction->x, 0.0f, direction->z);
        }


        bool ZombieCoreApproach::Approach(const ControllerContext& context, CharacterInput& input)
        {
            // コアがなければ、コアに着いていれば向かわない
            const std::optional<CoreApproachInfo> core = CalcToCore(context);
            if (!core || core->isArrived)
            {
                m_pathFollower.Clear();
                return false;
            }

            // 経路があれば経路に沿って、無ければコアへ真っ直ぐ向かう
            Vector3 direction(Vector3::Zero);
            if (const std::optional<Vector3> pathDirection = CalcPathDirection(context, core->corePosition))
            {
                direction = *pathDirection;
            }
            else
            {
                direction = core->toCore;
                direction.Normalize();
            }
            input.SetMove(direction);
            return true;
        }


        bool ZombieCoreApproach::IsArrived(const ControllerContext& context) const
        {
            const std::optional<CoreApproachInfo> core = CalcToCore(context);
            return core && core->isArrived;
        }
    } // namespace actor
} // namespace app
