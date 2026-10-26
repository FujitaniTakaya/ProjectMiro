/**
 * @file InGameInfoHub.cpp
 * @brief ゲーム中の情報を中継するクラス
 */
#include "stdafx.h"

#include "InGameInfoHub.h"

#include <utility>


namespace app
{
    InGameInfoHub::InGameInfoHub()
        : m_onPlayerHpChanged(nullptr)
        , m_hasPlayerHp(false)
        , m_playerHp(0.0f)
        , m_playerMaxHp(0.0f)
        , m_onPlayerAmmoChanged(nullptr)
        , m_hasPlayerAmmo(false)
        , m_playerAmmo(0)
        , m_onCoreHpChanged(nullptr)
        , m_hasCoreHp(false)
        , m_coreHp(0.0f)
        , m_coreMaxHp(0.0f)
    {}


    InGameInfoHub::~InGameInfoHub() = default;


    InGameInfoHub& InGameInfoHub::Get()
    {
        static InGameInfoHub instance;
        return instance;
    }


    void InGameInfoHub::SetOnPlayerHpChanged(PlayerHpCallback func)
    {
        m_onPlayerHpChanged = std::move(func);

        // 通知済みの最新値を、登録した側にも渡す
        if (m_onPlayerHpChanged && m_hasPlayerHp)
        {
            m_onPlayerHpChanged(m_playerHp, m_playerMaxHp);
        }
    }


    void InGameInfoHub::NotifyPlayerHp(const float hp, const float maxHp)
    {
        if (m_hasPlayerHp && m_playerHp == hp && m_playerMaxHp == maxHp)
        {
            return;
        }

        m_hasPlayerHp = true;
        m_playerHp = hp;
        m_playerMaxHp = maxHp;

        if (m_onPlayerHpChanged)
        {
            m_onPlayerHpChanged(m_playerHp, m_playerMaxHp);
        }
    }


    void InGameInfoHub::SetOnPlayerAmmoChanged(PlayerAmmoCallback func)
    {
        m_onPlayerAmmoChanged = std::move(func);

        // 通知済みの最新値を、登録した側にも渡す
        if (m_onPlayerAmmoChanged && m_hasPlayerAmmo)
        {
            m_onPlayerAmmoChanged(m_playerAmmo);
        }
    }


    void InGameInfoHub::NotifyPlayerAmmo(const uint32_t remainingAmmo)
    {
        if (m_hasPlayerAmmo && m_playerAmmo == remainingAmmo)
        {
            return;
        }

        m_hasPlayerAmmo = true;
        m_playerAmmo = remainingAmmo;

        if (m_onPlayerAmmoChanged)
        {
            m_onPlayerAmmoChanged(m_playerAmmo);
        }
    }


    void InGameInfoHub::SetOnCoreHpChanged(CoreHpCallback func)
    {
        m_onCoreHpChanged = std::move(func);

        // 通知済みの最新値を、登録した側にも渡す
        if (m_onCoreHpChanged && m_hasCoreHp)
        {
            m_onCoreHpChanged(m_coreHp, m_coreMaxHp);
        }
    }


    void InGameInfoHub::NotifyCoreHp(const float hp, const float maxHp)
    {
        if (m_hasCoreHp && m_coreHp == hp && m_coreMaxHp == maxHp)
        {
            return;
        }

        m_hasCoreHp = true;
        m_coreHp = hp;
        m_coreMaxHp = maxHp;

        if (m_onCoreHpChanged)
        {
            m_onCoreHpChanged(m_coreHp, m_coreMaxHp);
        }
    }


    void InGameInfoHub::ResetObservers()
    {
        m_onPlayerHpChanged = nullptr;
        m_hasPlayerHp = false;
        m_playerHp = 0.0f;
        m_playerMaxHp = 0.0f;
        m_onPlayerAmmoChanged = nullptr;
        m_hasPlayerAmmo = false;
        m_playerAmmo = 0;
        m_onCoreHpChanged = nullptr;
        m_hasCoreHp = false;
        m_coreHp = 0.0f;
        m_coreMaxHp = 0.0f;
    }
} // namespace app
