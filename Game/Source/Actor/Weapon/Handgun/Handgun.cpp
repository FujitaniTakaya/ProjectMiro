/**
 * @file Handgun.cpp
 * @brief ハンドガン
 */
#include "stdafx.h"

#include "Handgun.h"

#include "Source/Core/Param/ParamHolder.h"
#include "Source/Actor/Weapon/WeaponParameter.h"


namespace app
{
    namespace actor
    {
        Handgun::Handgun()
            : m_isReloading(false)
            , m_reloadTimer(0.0f)
        {}


        Handgun::~Handgun() = default;


        bool Handgun::Fire()
        {
            if (m_isReloading || m_remainingAmmo == 0)
            {
                return false;
            }
            --m_remainingAmmo;
            return true;
        }


        void Handgun::Reload()
        {
            if (m_isReloading || m_remainingAmmo >= m_magazineSize)
            {
                return;
            }

            if (m_reloadTime <= 0.0f)
            {
                m_remainingAmmo = m_magazineSize;
                return;
            }
            m_isReloading = true;
            m_reloadTimer = m_reloadTime;
        }


        void Handgun::Start()
        {
            // NOTE: パラメーターはコピーするので、jsonのホットリロードは次にStart()を呼ぶまで反映されない。
            Init(ParamHolder::Get().GetParameter<WeaponParameter>(EnParamID::Handgun));
            m_isReloading = false;
            m_reloadTimer = 0.0f;
        }


        void Handgun::Update()
        {
            if (!m_isReloading)
            {
                return;
            }

            m_reloadTimer -= g_gameTime->GetFrameDeltaTime();
            if (m_reloadTimer <= 0.0f)
            {
                m_isReloading = false;
                m_reloadTimer = 0.0f;
                m_remainingAmmo = m_magazineSize;
            }
        }


        void Handgun::Render(RenderContext& rc)
        {
            // モデルが無いので、何も描画しない。
        }
    } // namespace actor
} // namespace app
