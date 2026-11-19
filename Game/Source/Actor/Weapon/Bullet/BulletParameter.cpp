/**
 * @file BulletParameter.cpp
 * @brief Bulletのパラメーター
 */
#include "stdafx.h"

#include "BulletParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace actor
    {
        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        BulletParameter::BulletParameter()
            : lifeTime(3.0f)
            , shotSpeed(3000.0f)
            , modelScale(1.0f)
            , modelRotation(0.0f, 90.0f, 0.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadBulletParameter(const JsonView& root, BulletParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.lifeTime = ParamLoader::ToFloat(root, "lifeTime", param.lifeTime);
            param.shotSpeed = ParamLoader::ToFloat(root, "shotSpeed", param.shotSpeed);
            param.modelScale = ParamLoader::ToFloat(root, "modelScale", param.modelScale);
            param.modelRotation = ParamLoader::ToVector3(root, "modelRotation", param.modelRotation);
            // [PARAM_GEN end load]
        }


        void DrawBulletParameter([[maybe_unused]] const BulletParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("lifeTime: %.1f", param.lifeTime);
            ImGui::Text("shotSpeed: %.1f", param.shotSpeed);
            ImGui::Text("modelScale: %.1f", param.modelScale);
            ImGui::Text("modelRotation: (%.1f, %.1f, %.1f)", param.modelRotation.x, param.modelRotation.y, param.modelRotation.z);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
