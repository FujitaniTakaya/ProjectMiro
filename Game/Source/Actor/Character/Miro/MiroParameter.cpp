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
            , radius(25.0f)
            , height(110.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadMiroParameter(const JsonView& root, MiroParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.walkSpeed = ParamLoader::ToFloat(root, "walkSpeed", param.walkSpeed);
            param.runSpeed = ParamLoader::ToFloat(root, "runSpeed", param.runSpeed);
            param.radius = ParamLoader::ToFloat(root, "radius", param.radius);
            param.height = ParamLoader::ToFloat(root, "height", param.height);
            // [PARAM_GEN end load]
        }


        void DrawMiroParameter([[maybe_unused]] const MiroParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("walkSpeed: %.1f", param.walkSpeed);
            ImGui::Text("runSpeed: %.1f", param.runSpeed);
            ImGui::Text("radius: %.1f", param.radius);
            ImGui::Text("height: %.1f", param.height);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
