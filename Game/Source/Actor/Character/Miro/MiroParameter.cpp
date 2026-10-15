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
        namespace
        {
            /** 歩きの移動速度の初期値 */
            constexpr float DEFAULT_WALK_SPEED = 300.0f;
            /** 走りの移動速度の初期値 */
            constexpr float DEFAULT_RUN_SPEED = 600.0f;
            /** キャラクターコントローラーの半径の初期値 */
            constexpr float DEFAULT_RADIUS = 25.0f;
            /** キャラクターコントローラーの高さの初期値 */
            constexpr float DEFAULT_HEIGHT = 110.0f;
        } // namespace


        MiroParameter::MiroParameter()
            : walkSpeed(DEFAULT_WALK_SPEED)
            , runSpeed(DEFAULT_RUN_SPEED)
            , radius(DEFAULT_RADIUS)
            , height(DEFAULT_HEIGHT)
        {
        }




        /*************************************************************/


        void LoadMiroParameter(const JsonView& root, MiroParameter& param)
        {
            param.walkSpeed = ParamLoader::ToFloat(root, "walkSpeed", param.walkSpeed);
            param.runSpeed = ParamLoader::ToFloat(root, "runSpeed", param.runSpeed);
            param.radius = ParamLoader::ToFloat(root, "radius", param.radius);
            param.height = ParamLoader::ToFloat(root, "height", param.height);
        }


        void DrawMiroParameter([[maybe_unused]] const MiroParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            ImGui::Text("walkSpeed: %.1f", param.walkSpeed);
            ImGui::Text("runSpeed: %.1f", param.runSpeed);
            ImGui::Text("radius: %.1f", param.radius);
            ImGui::Text("height: %.1f", param.height);
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
