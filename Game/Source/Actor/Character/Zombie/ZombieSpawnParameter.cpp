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
            : gateNorth(2600.0f, 300.0f, -3100.0f)
            , gateSouth(-2600.0f, 300.0f, 2700.0f)
            , gateWest(-4400.0f, 0.0f, 0.0f)
            , gateEast(4400.0f, 0.0f, 0.0f)
            , spawnOrder("NNNR")
            , spawnInterval(3.0f)
            , maxAliveCount(20)
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
            param.spawnOrder = ParamLoader::ToString(root, "spawnOrder", param.spawnOrder);
            param.spawnInterval = ParamLoader::ToFloat(root, "spawnInterval", param.spawnInterval);
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
            ImGui::Text("spawnOrder: %s", param.spawnOrder.c_str());
            ImGui::Text("spawnInterval: %.1f", param.spawnInterval);
            ImGui::Text("maxAliveCount: %d", param.maxAliveCount);
            ImGui::Text("dropHeight: %.1f", param.dropHeight);
            ImGui::Text("gateJitterRadius: %.1f", param.gateJitterRadius);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
