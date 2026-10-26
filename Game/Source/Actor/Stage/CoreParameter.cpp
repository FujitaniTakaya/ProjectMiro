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
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadCoreParameter(const JsonView& root, CoreParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.maxHp = ParamLoader::ToFloat(root, "maxHp", param.maxHp);
            param.hpBarOffsetY = ParamLoader::ToFloat(root, "hpBarOffsetY", param.hpBarOffsetY);
            // [PARAM_GEN end load]
        }


        void DrawCoreParameter([[maybe_unused]] const CoreParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("maxHp: %.1f", param.maxHp);
            ImGui::Text("hpBarOffsetY: %.1f", param.hpBarOffsetY);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
