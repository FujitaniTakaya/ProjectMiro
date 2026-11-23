/**
 * @file ZombieSpawnParameter.cpp
 * @brief ZombieSpawnのパラメーター
 */
#include "stdafx.h"

#include "ZombieSpawnParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace actor
    {
        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        ZombieSpawnParameter::ZombieSpawnParameter()
            : gateNorth(2850.0f, 300.0f, -2900.0f)
            , gateSouth(-2850.0f, 300.0f, 2900.0f)
            , gateWest(-3650.0f, 0.0f, -1900.0f)
            , gateEast(3650.0f, 0.0f, 1900.0f)
            , groupMinCount(1)
            , groupMaxCount(3)
            , groupPeakCount(2)
            , intervalMinSeconds(6)
            , intervalMaxSeconds(10)
            , runningPeriod(4)
            , runningChancePercent(30.0f)
            , runningGuaranteedPeriod(20)
            , maxAliveCount(60)
            , dropHeight(200.0f)
            , gateJitterRadius(120.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        const Vector3& GetGatePosition(const ZombieSpawnParameter& param, const EnZombieGate gate)
        {
            switch (gate)
            {
            case EnZombieGate::North:
                return param.gateNorth;
            case EnZombieGate::South:
                return param.gateSouth;
            case EnZombieGate::West:
                return param.gateWest;
            case EnZombieGate::East:
                return param.gateEast;
            default:
                break;
            }

            // Max など、ゲートではない値。北を返して続行する
            K2_ASSERT(false, "ゲートではない値が渡されました。\n");
            return param.gateNorth;
        }


        void LoadZombieSpawnParameter(const JsonView& root, ZombieSpawnParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.gateNorth = ParamLoader::ToVector3(root, "gateNorth", param.gateNorth);
            param.gateSouth = ParamLoader::ToVector3(root, "gateSouth", param.gateSouth);
            param.gateWest = ParamLoader::ToVector3(root, "gateWest", param.gateWest);
            param.gateEast = ParamLoader::ToVector3(root, "gateEast", param.gateEast);
            param.groupMinCount = ParamLoader::ToInt(root, "groupMinCount", param.groupMinCount);
            param.groupMaxCount = ParamLoader::ToInt(root, "groupMaxCount", param.groupMaxCount);
            param.groupPeakCount = ParamLoader::ToInt(root, "groupPeakCount", param.groupPeakCount);
            param.intervalMinSeconds = ParamLoader::ToInt(root, "intervalMinSeconds", param.intervalMinSeconds);
            param.intervalMaxSeconds = ParamLoader::ToInt(root, "intervalMaxSeconds", param.intervalMaxSeconds);
            param.runningPeriod = ParamLoader::ToInt(root, "runningPeriod", param.runningPeriod);
            param.runningChancePercent = ParamLoader::ToFloat(root, "runningChancePercent", param.runningChancePercent);
            param.runningGuaranteedPeriod = ParamLoader::ToInt(root, "runningGuaranteedPeriod", param.runningGuaranteedPeriod);
            param.maxAliveCount = ParamLoader::ToInt(root, "maxAliveCount", param.maxAliveCount);
            param.dropHeight = ParamLoader::ToFloat(root, "dropHeight", param.dropHeight);
            param.gateJitterRadius = ParamLoader::ToFloat(root, "gateJitterRadius", param.gateJitterRadius);
            // [PARAM_GEN end load]
        }


        void DrawZombieSpawnParameter([[maybe_unused]] const ZombieSpawnParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("gateNorth: (%.1f, %.1f, %.1f)", param.gateNorth.x, param.gateNorth.y, param.gateNorth.z);
            ImGui::Text("gateSouth: (%.1f, %.1f, %.1f)", param.gateSouth.x, param.gateSouth.y, param.gateSouth.z);
            ImGui::Text("gateWest: (%.1f, %.1f, %.1f)", param.gateWest.x, param.gateWest.y, param.gateWest.z);
            ImGui::Text("gateEast: (%.1f, %.1f, %.1f)", param.gateEast.x, param.gateEast.y, param.gateEast.z);
            ImGui::Text("groupMinCount: %d", param.groupMinCount);
            ImGui::Text("groupMaxCount: %d", param.groupMaxCount);
            ImGui::Text("groupPeakCount: %d", param.groupPeakCount);
            ImGui::Text("intervalMinSeconds: %d", param.intervalMinSeconds);
            ImGui::Text("intervalMaxSeconds: %d", param.intervalMaxSeconds);
            ImGui::Text("runningPeriod: %d", param.runningPeriod);
            ImGui::Text("runningChancePercent: %.1f", param.runningChancePercent);
            ImGui::Text("runningGuaranteedPeriod: %d", param.runningGuaranteedPeriod);
            ImGui::Text("maxAliveCount: %d", param.maxAliveCount);
            ImGui::Text("dropHeight: %.1f", param.dropHeight);
            ImGui::Text("gateJitterRadius: %.1f", param.gateJitterRadius);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
