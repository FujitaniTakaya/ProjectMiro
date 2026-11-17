/**
 * @file MiroParameter.cpp
 * @brief Miroのパラメーター
 */
#include "stdafx.h"

#include "MiroParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace actor
    {
        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        MiroParameter::MiroParameter()
            : walkSpeed(300.0f)
            , runSpeed(600.0f)
            , aimMoveSpeed(150.0f)
            , aimTurnDegrees(20.0f)
            , upperBodyBlendTime(0.15f)
            , aimHipFollow(0.6f)
            , radius(25.0f)
            , height(110.0f)
            , maxHp(100.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadMiroParameter(const JsonView& root, MiroParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.walkSpeed = ParamLoader::ToFloat(root, "walkSpeed", param.walkSpeed);
            param.runSpeed = ParamLoader::ToFloat(root, "runSpeed", param.runSpeed);
            param.aimMoveSpeed = ParamLoader::ToFloat(root, "aimMoveSpeed", param.aimMoveSpeed);
            param.aimTurnDegrees = ParamLoader::ToFloat(root, "aimTurnDegrees", param.aimTurnDegrees);
            param.upperBodyBlendTime = ParamLoader::ToFloat(root, "upperBodyBlendTime", param.upperBodyBlendTime);
            param.aimHipFollow = ParamLoader::ToFloat(root, "aimHipFollow", param.aimHipFollow);
            param.radius = ParamLoader::ToFloat(root, "radius", param.radius);
            param.height = ParamLoader::ToFloat(root, "height", param.height);
            param.maxHp = ParamLoader::ToFloat(root, "maxHp", param.maxHp);
            // [PARAM_GEN end load]
        }


        void DrawMiroParameter([[maybe_unused]] const MiroParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("walkSpeed: %.1f", param.walkSpeed);
            ImGui::Text("runSpeed: %.1f", param.runSpeed);
            ImGui::Text("aimMoveSpeed: %.1f", param.aimMoveSpeed);
            ImGui::Text("aimTurnDegrees: %.1f", param.aimTurnDegrees);
            ImGui::Text("upperBodyBlendTime: %.2f", param.upperBodyBlendTime);
            ImGui::Text("aimHipFollow: %.2f", param.aimHipFollow);
            ImGui::Text("radius: %.1f", param.radius);
            ImGui::Text("height: %.1f", param.height);
            ImGui::Text("maxHp: %.1f", param.maxHp);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
