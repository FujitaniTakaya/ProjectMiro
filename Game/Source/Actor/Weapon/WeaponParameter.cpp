/**
 * @file WeaponParameter.cpp
 * @brief ウェポンのパラメーター
 */
#include "stdafx.h"

#include "WeaponParameter.h"

#include "Source/Core/Json/ParamLoader.h"


namespace app
{
    namespace actor
    {
        WeaponParameter::WeaponParameter()
            : reloadTime(0.0f)
            , magazineSize(0)
            , gripPosition(Vector3::Zero)
            , gripRotation(Quaternion::Identity)
        {
        }




        /*************************************************************/


        void LoadWeaponParameter(const JsonView& root, WeaponParameter& param)
        {
            param.reloadTime = ParamLoader::ToFloat(root, "reloadTime", param.reloadTime);
            param.magazineSize = ParamLoader::ToUInt32(root, "magazineSize", param.magazineSize);
            param.gripPosition = ParamLoader::ToVector3(root, "gripPosition", param.gripPosition);
            param.gripRotation = ParamLoader::ToRotation(root, "gripRotation", true, param.gripRotation);
        }


        void DrawWeaponParameter([[maybe_unused]] const WeaponParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            ImGui::Text("reloadTime: %.2f", param.reloadTime);
            ImGui::Text("magazineSize: %u", param.magazineSize);
            ImGui::Text("gripPosition: (%.2f, %.2f, %.2f)", param.gripPosition.x, param.gripPosition.y, param.gripPosition.z);
            ImGui::Text("gripRotation: (%.3f, %.3f, %.3f, %.3f)", param.gripRotation.x, param.gripRotation.y, param.gripRotation.z, param.gripRotation.w);
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
