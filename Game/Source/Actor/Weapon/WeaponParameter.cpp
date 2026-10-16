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
        {
        }




        /*************************************************************/


        void LoadWeaponParameter(const JsonView& root, WeaponParameter& param)
        {
            param.reloadTime = ParamLoader::ToFloat(root, "reloadTime", param.reloadTime);
            param.magazineSize = ParamLoader::ToUInt32(root, "magazineSize", param.magazineSize);
        }


        void DrawWeaponParameter([[maybe_unused]] const WeaponParameter& param)
        {
#ifdef BALLOON_IMGUI_ENABLED
            ImGui::Text("reloadTime: %.2f", param.reloadTime);
            ImGui::Text("magazineSize: %u", param.magazineSize);
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace actor
} // namespace app
