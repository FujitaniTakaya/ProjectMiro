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
            , muzzleBoneName()
            , muzzlePosition(Vector3::Zero)
            , muzzleRotation(Quaternion::Identity)
        {
        }




        /*************************************************************/


        void LoadWeaponParameter(const JsonView& root, WeaponParameter& param)
        {
            param.reloadTime = ParamLoader::ToFloat(root, "reloadTime", param.reloadTime);
            param.magazineSize = ParamLoader::ToUInt32(root, "magazineSize", param.magazineSize);
            param.gripPosition = ParamLoader::ToVector3(root, "gripPosition", param.gripPosition);
            param.gripRotation = ParamLoader::ToRotation(root, "gripRotation", true, param.gripRotation);
            param.muzzleBoneName = ParamLoader::ToString(root, "muzzleBoneName", param.muzzleBoneName);
            param.muzzlePosition = ParamLoader::ToVector3(root, "muzzlePosition", param.muzzlePosition);
            param.muzzleRotation = ParamLoader::ToRotation(root, "muzzleRotation", true, param.muzzleRotation);
        }


        void DrawWeaponParameter([[maybe_unused]] const WeaponParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            ImGui::Text("reloadTime: %.2f", param.reloadTime);
            ImGui::Text("magazineSize: %u", param.magazineSize);
            ImGui::Text("gripPosition: (%.2f, %.2f, %.2f)", param.gripPosition.x, param.gripPosition.y, param.gripPosition.z);
            ImGui::Text("gripRotation: (%.3f, %.3f, %.3f, %.3f)", param.gripRotation.x, param.gripRotation.y, param.gripRotation.z, param.gripRotation.w);
            ImGui::Text("muzzleBoneName: %s", param.muzzleBoneName.c_str());
            ImGui::Text("muzzlePosition: (%.2f, %.2f, %.2f)", param.muzzlePosition.x, param.muzzlePosition.y, param.muzzlePosition.z);
            ImGui::Text("muzzleRotation: (%.3f, %.3f, %.3f, %.3f)", param.muzzleRotation.x, param.muzzleRotation.y, param.muzzleRotation.z, param.muzzleRotation.w);
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
