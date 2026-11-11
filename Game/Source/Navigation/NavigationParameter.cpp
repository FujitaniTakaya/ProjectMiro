/**
 * @file NavigationParameter.cpp
 * @brief Navigationのパラメーター
 */
#include "stdafx.h"

#include "NavigationParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace nav
    {
        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        NavigationParameter::NavigationParameter()
            : cellSize(25.0f)
            , maxSlopeDegrees(45.0f)
            , maxStep(35.0f)
            , agentRadius(25.0f)
            , agentHeight(110.0f)
            , snapRadiusCells(8)
            , reachRadius(60.0f)
            , maxDeviation(200.0f)
            , stallTime(1.5f)
            , stallDistance(20.0f)
            , repathInterval(1.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadNavigationParameter(const JsonView& root, NavigationParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.cellSize = ParamLoader::ToFloat(root, "cellSize", param.cellSize);
            param.maxSlopeDegrees = ParamLoader::ToFloat(root, "maxSlopeDegrees", param.maxSlopeDegrees);
            param.maxStep = ParamLoader::ToFloat(root, "maxStep", param.maxStep);
            param.agentRadius = ParamLoader::ToFloat(root, "agentRadius", param.agentRadius);
            param.agentHeight = ParamLoader::ToFloat(root, "agentHeight", param.agentHeight);
            param.snapRadiusCells = ParamLoader::ToInt(root, "snapRadiusCells", param.snapRadiusCells);
            param.reachRadius = ParamLoader::ToFloat(root, "reachRadius", param.reachRadius);
            param.maxDeviation = ParamLoader::ToFloat(root, "maxDeviation", param.maxDeviation);
            param.stallTime = ParamLoader::ToFloat(root, "stallTime", param.stallTime);
            param.stallDistance = ParamLoader::ToFloat(root, "stallDistance", param.stallDistance);
            param.repathInterval = ParamLoader::ToFloat(root, "repathInterval", param.repathInterval);
            // [PARAM_GEN end load]
        }


        void DrawNavigationParameter([[maybe_unused]] const NavigationParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("cellSize: %.1f", param.cellSize);
            ImGui::Text("maxSlopeDegrees: %.1f", param.maxSlopeDegrees);
            ImGui::Text("maxStep: %.1f", param.maxStep);
            ImGui::Text("agentRadius: %.1f", param.agentRadius);
            ImGui::Text("agentHeight: %.1f", param.agentHeight);
            ImGui::Text("snapRadiusCells: %d", param.snapRadiusCells);
            ImGui::Text("reachRadius: %.1f", param.reachRadius);
            ImGui::Text("maxDeviation: %.1f", param.maxDeviation);
            ImGui::Text("stallTime: %.2f", param.stallTime);
            ImGui::Text("stallDistance: %.1f", param.stallDistance);
            ImGui::Text("repathInterval: %.2f", param.repathInterval);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace nav
} // namespace app
