/**
 * @file WeaponBase.cpp
 * @brief ウェポンの基底クラス
 */
#include "stdafx.h"

#include "WeaponBase.h"

#include "WeaponParameter.h"


namespace app
{
    namespace actor
    {
        WeaponBase::WeaponBase()
            : m_reloadTime(0.0f)
            , m_magazineSize(0)
            , m_remainingAmmo(0)
        {
        }


        void WeaponBase::Init(const WeaponParameter& param)
        {
            m_reloadTime = param.reloadTime;
            m_magazineSize = param.magazineSize;
            m_remainingAmmo = m_magazineSize;
        }
    } // namespace actor
} // namespace app
