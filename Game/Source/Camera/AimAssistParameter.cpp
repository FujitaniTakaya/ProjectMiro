/**
 * @file AimAssistParameter.cpp
 * @brief AimAssistのパラメーター
 */
#include "stdafx.h"

#include "AimAssistParameter.h"

#include <algorithm>

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace camera
    {
        namespace
        {
            /** 減速の倍率の下限。これより小さいと、ほぼ回らなくなる */
            constexpr float MIN_SLOW_SCALE = 0.05f;
        } // namespace


        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        AimAssistParameter::AimAssistParameter()
            : targetHeight(120.0f)
            , maxDistance(3000.0f)
            , assistRadius(80.0f)
            , slowScale(0.4f)
            , pullRate(2.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadAimAssistParameter(const JsonView& root, AimAssistParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.targetHeight = ParamLoader::ToFloat(root, "targetHeight", param.targetHeight);
            param.maxDistance = ParamLoader::ToFloat(root, "maxDistance", param.maxDistance);
            param.assistRadius = ParamLoader::ToFloat(root, "assistRadius", param.assistRadius);
            param.pullRate = ParamLoader::ToFloat(root, "pullRate", param.pullRate);
            // [PARAM_GEN end load]

            // 使えない値は、K2_LOGを出して初期値に戻す。
            // 距離・半径は、0以下だと対象が無くなり(割り算も0になる)、引き寄せの強さは、負だと逆に回る。
            if (param.maxDistance <= 0.0f || param.assistRadius <= 0.0f || param.pullRate < 0.0f)
            {
                K2_LOG("AimAssistの\"maxDistance\"/\"assistRadius\"は0より大きく、\"pullRate\"は0以上にしてください。初期値を使います。\n");
                const AimAssistParameter defaults;
                param.maxDistance = defaults.maxDistance;
                param.assistRadius = defaults.assistRadius;
                param.pullRate = defaults.pullRate;
            }

            // 回転速度の倍率。0に近いと回らなくなるので、下限を決める。1より大きいと、加速になってしまう。
            param.slowScale = std::clamp(ParamLoader::ToFloat(root, "slowScale", param.slowScale), MIN_SLOW_SCALE, 1.0f);
        }


        void DrawAimAssistParameter([[maybe_unused]] const AimAssistParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("targetHeight: %.1f", param.targetHeight);
            ImGui::Text("maxDistance: %.1f", param.maxDistance);
            ImGui::Text("assistRadius: %.1f", param.assistRadius);
            ImGui::Text("slowScale: %.2f", param.slowScale);
            ImGui::Text("pullRate: %.1f", param.pullRate);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace camera
} // namespace app
