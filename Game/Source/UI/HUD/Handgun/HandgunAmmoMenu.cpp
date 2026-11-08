/**
 * @file HandgunAmmoMenu.cpp
 * @brief ハンドガンの残段数UIを動かすMenu
 */
#include "stdafx.h"

#include "HandgunAmmoMenu.h"

#include <cstdint>
#include <string>

#include "Source/Core/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 残段数の数字のUIの名前(ハッシュ値) */
            constexpr uint32_t AMMO_NUMBER_KEY = Hash32("AmmoNumber");
        } // namespace


        HandgunAmmoMenu::HandgunAmmoMenu()
            : m_ammoNumber(nullptr)
            , m_remainingAmmo(0)
        {}


        HandgunAmmoMenu::~HandgunAmmoMenu() = default;


        void HandgunAmmoMenu::InitializeLogic()
        {
            m_ammoNumber = GetUI<UIText>(AMMO_NUMBER_KEY);
            if (!m_ammoNumber)
            {
                K2_LOG("HandgunAmmoMenu: UIText \"AmmoNumber\" がありません。\n");
            }

            ApplyAmmo();
        }


        void HandgunAmmoMenu::SetAmmo(const uint32_t remainingAmmo)
        {
            m_remainingAmmo = remainingAmmo;
            ApplyAmmo();
        }


        void HandgunAmmoMenu::ApplyAmmo()
        {
            if (m_ammoNumber)
            {
                m_ammoNumber->SetText(std::to_wstring(m_remainingAmmo).c_str());
            }
        }
    } // namespace ui
} // namespace app
