/**
 * @file CoreParameter.cpp
 * @brief Coreのパラメーター
 */
#include "stdafx.h"

#include "CoreParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace actor
    {
        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        CoreParameter::CoreParameter()
            : maxHp(1000.0f)
            , hpBarOffsetY(150.0f)
            , hitBoxWidth(240.0f)
            , hitBoxHeight(220.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadCoreParameter(const JsonView& root, CoreParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.maxHp = ParamLoader::ToFloat(root, "maxHp", param.maxHp);
            param.hpBarOffsetY = ParamLoader::ToFloat(root, "hpBarOffsetY", param.hpBarOffsetY);
            param.hitBoxWidth = ParamLoader::ToFloat(root, "hitBoxWidth", param.hitBoxWidth);
            param.hitBoxHeight = ParamLoader::ToFloat(root, "hitBoxHeight", param.hitBoxHeight);
            // [PARAM_GEN end load]
        }


        void DrawCoreParameter([[maybe_unused]] const CoreParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("maxHp: %.1f", param.maxHp);
            ImGui::Text("hpBarOffsetY: %.1f", param.hpBarOffsetY);
            ImGui::Text("hitBoxWidth: %.1f", param.hitBoxWidth);
            ImGui::Text("hitBoxHeight: %.1f", param.hitBoxHeight);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
