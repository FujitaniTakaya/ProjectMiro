/**
 * @file ZombieParameter.cpp
 * @brief Zombieのパラメーター
 */
#include "stdafx.h"

#include "ZombieParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace actor
    {
        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        ZombieParameter::ZombieParameter()
            : maxHp(50.0f)
            , walkSpeed(0.3f)
            , runSpeed(0.9f)
            , attackPower(10.0f)
            , initialAnger(0.0f)
            , angerGainRate(1.0f)
            , angerThreshold(100.0f)
            , angerSpreadValue(30.0f)
            , angerSpreadRadius(500.0f)
            , radius(25.0f)
            , height(110.0f)
            , coreStopDistance(150.0f)
            , attackBoxSize(60.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadZombieParameter(const JsonView& root, ZombieParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.maxHp = ParamLoader::ToFloat(root, "maxHp", param.maxHp);
            param.walkSpeed = ParamLoader::ToFloat(root, "walkSpeed", param.walkSpeed);
            param.runSpeed = ParamLoader::ToFloat(root, "runSpeed", param.runSpeed);
            param.attackPower = ParamLoader::ToFloat(root, "attackPower", param.attackPower);
            param.initialAnger = ParamLoader::ToFloat(root, "initialAnger", param.initialAnger);
            param.angerGainRate = ParamLoader::ToFloat(root, "angerGainRate", param.angerGainRate);
            param.angerThreshold = ParamLoader::ToFloat(root, "angerThreshold", param.angerThreshold);
            param.angerSpreadValue = ParamLoader::ToFloat(root, "angerSpreadValue", param.angerSpreadValue);
            param.angerSpreadRadius = ParamLoader::ToFloat(root, "angerSpreadRadius", param.angerSpreadRadius);
            param.radius = ParamLoader::ToFloat(root, "radius", param.radius);
            param.height = ParamLoader::ToFloat(root, "height", param.height);
            param.coreStopDistance = ParamLoader::ToFloat(root, "coreStopDistance", param.coreStopDistance);
            param.attackBoxSize = ParamLoader::ToFloat(root, "attackBoxSize", param.attackBoxSize);
            // [PARAM_GEN end load]
        }


        void DrawZombieParameter([[maybe_unused]] const ZombieParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("maxHp: %.1f", param.maxHp);
            ImGui::Text("walkSpeed: %.2f", param.walkSpeed);
            ImGui::Text("runSpeed: %.2f", param.runSpeed);
            ImGui::Text("attackPower: %.1f", param.attackPower);
            ImGui::Text("initialAnger: %.1f", param.initialAnger);
            ImGui::Text("angerGainRate: %.2f", param.angerGainRate);
            ImGui::Text("angerThreshold: %.1f", param.angerThreshold);
            ImGui::Text("angerSpreadValue: %.1f", param.angerSpreadValue);
            ImGui::Text("angerSpreadRadius: %.1f", param.angerSpreadRadius);
            ImGui::Text("radius: %.1f", param.radius);
            ImGui::Text("height: %.1f", param.height);
            ImGui::Text("coreStopDistance: %.1f", param.coreStopDistance);
            ImGui::Text("attackBoxSize: %.1f", param.attackBoxSize);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
