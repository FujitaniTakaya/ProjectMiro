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
            /** 注視点を上にずらす量の初期値 */
            constexpr float DEFAULT_TARGET_OFFSET_UP = 80.0f;
            /** 注視点を右にずらす量の初期値(仮の値。実機を見て調整する) */
            constexpr float DEFAULT_TARGET_OFFSET_RIGHT = 60.0f;
            /** 注視点から視点までのベクトルの初期値 */
            constexpr float DEFAULT_TO_CAMERA_POS_X = 0.0f;
            constexpr float DEFAULT_TO_CAMERA_POS_Y = 125.0f;
            constexpr float DEFAULT_TO_CAMERA_POS_Z = -250.0f;
            /** 回転速度の初期値(度/秒。1フレームあたり1.3度を60fpsで回す速さ) */
            constexpr float DEFAULT_ROTATE_SPEED = 78.0f;
            /** y成分の下限の初期値 */
            constexpr float DEFAULT_MIN_TO_CAMERA_DIR_Y = -0.2f;
            /** y成分の上限の初期値 */
            constexpr float DEFAULT_MAX_TO_CAMERA_DIR_Y = 0.9f;
            /** ニアクリップの初期値 */
            constexpr float DEFAULT_NEAR_CLIP = 1.0f;
            /** ファークリップの初期値 */
            constexpr float DEFAULT_FAR_CLIP = 10000.0f;

            /**
             * y成分の制限に使える絶対値の上限
             * 真上・真下(1.0)だと、X軸回りの回転軸 Cross(AxisY, v) が0になってしまうため、手前で止める。
             */
            constexpr float DIR_Y_LIMIT = 0.99f;
        } // namespace


        PlayerCameraParameter::PlayerCameraParameter()
            : targetOffsetUp(DEFAULT_TARGET_OFFSET_UP)
            , targetOffsetRight(DEFAULT_TARGET_OFFSET_RIGHT)
            , toCameraPos(DEFAULT_TO_CAMERA_POS_X, DEFAULT_TO_CAMERA_POS_Y, DEFAULT_TO_CAMERA_POS_Z)
            , rotateSpeed(DEFAULT_ROTATE_SPEED)
            , minToCameraDirY(DEFAULT_MIN_TO_CAMERA_DIR_Y)
            , maxToCameraDirY(DEFAULT_MAX_TO_CAMERA_DIR_Y)
            , nearClip(DEFAULT_NEAR_CLIP)
            , farClip(DEFAULT_FAR_CLIP)
        {
        }




        /*************************************************************/


        void LoadPlayerCameraParameter(const JsonView& root, PlayerCameraParameter& param)
        {
            param.targetOffsetUp = ParamLoader::ToFloat(root, "targetOffsetUp", param.targetOffsetUp);
            param.targetOffsetRight = ParamLoader::ToFloat(root, "targetOffsetRight", param.targetOffsetRight);
            param.rotateSpeed = ParamLoader::ToFloat(root, "rotateSpeed", param.rotateSpeed);

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
            ImGui::Text("targetOffsetUp: %.1f", param.targetOffsetUp);
            ImGui::Text("targetOffsetRight: %.1f", param.targetOffsetRight);
            ImGui::Text("toCameraPos: (%.1f, %.1f, %.1f)", param.toCameraPos.x, param.toCameraPos.y, param.toCameraPos.z);
            ImGui::Text("rotateSpeed: %.1f", param.rotateSpeed);
            ImGui::Text("minToCameraDirY: %.2f", param.minToCameraDirY);
            ImGui::Text("maxToCameraDirY: %.2f", param.maxToCameraDirY);
            ImGui::Text("nearClip: %.1f", param.nearClip);
            ImGui::Text("farClip: %.1f", param.farClip);
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace camera
} // namespace app
