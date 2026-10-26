/**
 * @file PlayerCameraParameter.cpp
 * @brief PlayerCameraのパラメーター
 */
#include "stdafx.h"

#include "PlayerCameraParameter.h"

#include <algorithm>
#include <cfloat>

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace camera
    {
        namespace
        {
            /**
             * y成分の制限に使える絶対値の上限
             * 真上・真下(1.0)だと、X軸回りの回転軸 Cross(AxisY, v) が0になってしまうため、手前で止める。
             */
            constexpr float DIR_Y_LIMIT = 0.99f;
        } // namespace


        // [PARAM_GEN begin ctor] 自動生成(tools/param_gen.py)。手で編集しない
        PlayerCameraParameter::PlayerCameraParameter()
            : targetOffsetUp(180.0f)
            , targetOffsetRight(60.0f)
            , toCameraPos(0.0f, 125.0f, -250.0f)
            , rotateSpeed(78.0f)
            , minToCameraDirY(-0.2f)
            , maxToCameraDirY(0.9f)
            , nearClip(1.0f)
            , farClip(10000.0f)
        {
        }
        // [PARAM_GEN end ctor]




        /*************************************************************/


        void LoadPlayerCameraParameter(const JsonView& root, PlayerCameraParameter& param)
        {
            // [PARAM_GEN begin load] 自動生成(tools/param_gen.py)。手で編集しない
            param.targetOffsetUp = ParamLoader::ToFloat(root, "targetOffsetUp", param.targetOffsetUp);
            param.targetOffsetRight = ParamLoader::ToFloat(root, "targetOffsetRight", param.targetOffsetRight);
            param.rotateSpeed = ParamLoader::ToFloat(root, "rotateSpeed", param.rotateSpeed);
            // [PARAM_GEN end load]

            // 注視点から視点までのベクトル。水平成分が0だと、カメラの右方向が決まらない。
            const Vector3 toCameraPos = ParamLoader::ToVector3(root, "toCameraPos", param.toCameraPos);
            if (toCameraPos.x * toCameraPos.x + toCameraPos.z * toCameraPos.z > FLT_EPSILON)
            {
                param.toCameraPos = toCameraPos;
            }
            else
            {
                K2_LOG("PlayerCameraの\"toCameraPos\"は、水平方向(x, z)が0です。初期値を使います。\n");
            }

            // 上下の制限。範囲内に収めて、下限が上限より小さいときだけ使う。
            const float minDirY = std::clamp(ParamLoader::ToFloat(root, "minToCameraDirY", param.minToCameraDirY), -DIR_Y_LIMIT, DIR_Y_LIMIT);
            const float maxDirY = std::clamp(ParamLoader::ToFloat(root, "maxToCameraDirY", param.maxToCameraDirY), -DIR_Y_LIMIT, DIR_Y_LIMIT);
            if (minDirY < maxDirY)
            {
                param.minToCameraDirY = minDirY;
                param.maxToCameraDirY = maxDirY;
            }
            else
            {
                K2_LOG("PlayerCameraの\"minToCameraDirY\"が\"maxToCameraDirY\"以上です。初期値を使います。\n");
            }

            // クリップ。ニアは0より大きく、ファーはニアより大きいときだけ使う。
            const float nearClip = ParamLoader::ToFloat(root, "nearClip", param.nearClip);
            const float farClip = ParamLoader::ToFloat(root, "farClip", param.farClip);
            if (nearClip > 0.0f && farClip > nearClip)
            {
                param.nearClip = nearClip;
                param.farClip = farClip;
            }
            else
            {
                K2_LOG("PlayerCameraの\"nearClip\"/\"farClip\"が使えない値です。初期値を使います。\n");
            }
        }


        void DrawPlayerCameraParameter([[maybe_unused]] const PlayerCameraParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            // [PARAM_GEN begin draw] 自動生成(tools/param_gen.py)。手で編集しない
            ImGui::Text("targetOffsetUp: %.1f", param.targetOffsetUp);
            ImGui::Text("targetOffsetRight: %.1f", param.targetOffsetRight);
            ImGui::Text("toCameraPos: (%.1f, %.1f, %.1f)", param.toCameraPos.x, param.toCameraPos.y, param.toCameraPos.z);
            ImGui::Text("rotateSpeed: %.1f", param.rotateSpeed);
            ImGui::Text("minToCameraDirY: %.2f", param.minToCameraDirY);
            ImGui::Text("maxToCameraDirY: %.2f", param.maxToCameraDirY);
            ImGui::Text("nearClip: %.1f", param.nearClip);
            ImGui::Text("farClip: %.1f", param.farClip);
            // [PARAM_GEN end draw]
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace camera
} // namespace app
