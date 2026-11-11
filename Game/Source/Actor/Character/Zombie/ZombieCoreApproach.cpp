/**
 * @file ZombieCoreApproach.cpp
 * @brief ゾンビがコアへ向かう処理
 */
#include "stdafx.h"

#include "ZombieCoreApproach.h"

#include <algorithm>
#include <vector>

#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
#include "Source/Navigation/NavigationParameter.h"
#include "Source/Navigation/NavigationService.h"
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


        bool ZombieCoreApproach::CalcToCore(const ControllerContext& context, Vector3& outCorePosition, Vector3& outToCore, bool& outIsArrived) const
        {
            // コアがなければ求められない
            Vector3 corePosition(Vector3::Zero);
            if (!m_corePositionProvider || !m_corePositionProvider(corePosition))
            {
                return false;
            }
            outCorePosition = corePosition;

            // 高さは無視して、XZ平面で測る
            outToCore = Vector3(corePosition.x - context.position.x, 0.0f, corePosition.z - context.position.z);
            const float distanceSq = outToCore.LengthSq();

            // コアに着いたとみなす。真上・真下にいて向きが決まらないときも、着いた扱いにする
            const float stopDistance = m_parameter.coreStopDistance;
            outIsArrived = distanceSq <= stopDistance * stopDistance || distanceSq <= DIRECTION_NORMALIZE_SQ;
            return true;
        }


        bool ZombieCoreApproach::CalcPathDirection(const ControllerContext& context, const Vector3& corePosition, Vector3& outDirection)
        {
            nav::NavigationService& navigation = nav::NavigationService::Get();
            if (!navigation.IsReady())
            {
                m_pathFollower.Clear();
                return false;
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
                std::vector<nav::NavVec3> path;
                if (navigation.FindPath(context.position, corePosition, path))
                {
                    m_pathFollower.SetPath(path);
                }
                else
                {
                    m_pathFollower.Clear();
                }
            }

            float directionX = 0.0f;
            float directionZ = 0.0f;
            if (!m_pathFollower.GetMoveDirection(context.position.x, context.position.z, directionX, directionZ))
            {
                return false;
            }
            outDirection = Vector3(directionX, 0.0f, directionZ);
            return true;
        }


        bool ZombieCoreApproach::Approach(const ControllerContext& context, CharacterInput& input)
        {
            // コアがなければ、コアに着いていれば向かわない
            Vector3 corePosition(Vector3::Zero);
            Vector3 toCore(Vector3::Zero);
            bool isArrived = false;
            if (!CalcToCore(context, corePosition, toCore, isArrived) || isArrived)
            {
                m_pathFollower.Clear();
                return false;
            }

            // 経路があれば経路に沿って、無ければコアへ真っ直ぐ向かう
            Vector3 direction(Vector3::Zero);
            if (!CalcPathDirection(context, corePosition, direction))
            {
                direction = toCore;
                direction.Normalize();
            }
            input.SetMove(direction);
            return true;
        }


        bool ZombieCoreApproach::IsArrived(const ControllerContext& context) const
        {
            Vector3 corePosition(Vector3::Zero);
            Vector3 toCore(Vector3::Zero);
            bool isArrived = false;
            return CalcToCore(context, corePosition, toCore, isArrived) && isArrived;
        }
    } // namespace actor
} // namespace app
