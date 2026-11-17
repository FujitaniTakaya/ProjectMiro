/**
 * @file NavigationService.cpp
 * @brief 経路探索のサービス
 */
#include "stdafx.h"

#include "NavigationService.h"

#include "NavGridBuilder.h"
#include "NavigationParameter.h"
#include "PathFinder.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Param/ParamList.h"
#include "StageTriangleSource.h"


namespace app
{
    namespace nav
    {
        NavigationService::NavigationService()
            : m_grid()
        {
        }


        NavigationService& NavigationService::Get()
        {
            static NavigationService instance;
            return instance;
        }


        bool NavigationService::Build(const char* tkmPath, const Matrix& modelToWorld)
        {
            m_grid = NavGrid();

            std::vector<NavTriangle> triangles;
            if (!LoadStageTriangles(tkmPath, modelToWorld, triangles) || triangles.empty())
            {
                K2_LOG("NavigationService: ステージの三角形を取り出せなかった。path=%s\n", tkmPath);
                return false;
            }

            const NavigationParameter& parameter = ParamHolder::Get().GetParameter<NavigationParameter>(EnParamID::Navigation);
            NavBuildSettings settings;
            settings.cellSize = parameter.cellSize;
            settings.maxSlopeDegrees = parameter.maxSlopeDegrees;
            settings.maxStep = parameter.maxStep;
            settings.agentRadius = parameter.agentRadius;
            settings.agentHeight = parameter.agentHeight;
            m_grid = BuildNavGrid(triangles, settings);
            return !m_grid.IsEmpty();
        }


        void NavigationService::Clear()
        {
            m_grid = NavGrid();
        }


        bool NavigationService::IsReady() const
        {
            return !m_grid.IsEmpty();
        }


        std::optional<std::vector<NavVec3>> NavigationService::FindPath(const Vector3& start, const Vector3& goal) const
        {
            if (m_grid.IsEmpty())
            {
                return std::nullopt;
            }

            const NavigationParameter& parameter = ParamHolder::Get().GetParameter<NavigationParameter>(EnParamID::Navigation);
            PathFindSettings settings;
            settings.maxStep = parameter.maxStep;
            settings.snapRadiusCells = parameter.snapRadiusCells;
            return nav::FindPath(m_grid, NavVec3{ start.x, start.y, start.z }, NavVec3{ goal.x, goal.y, goal.z }, settings);
        }


        const NavGrid& NavigationService::GetGrid() const
        {
            return m_grid;
        }
    } // namespace nav
} // namespace app
