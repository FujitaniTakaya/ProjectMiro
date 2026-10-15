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


    void InGameInfoHub::ResetObservers()
    {
        m_onPlayerHpChanged = nullptr;
        m_hasPlayerHp = false;
        m_playerHp = 0.0f;
        m_playerMaxHp = 0.0f;
    }
} // namespace app
