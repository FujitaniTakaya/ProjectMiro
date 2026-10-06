/**
 * @file VolumeFader.cpp
 * @brief 音量の割合を、徐々に変えるクラス
 */
#include "stdafx.h"

#include "VolumeFader.h"


namespace app
{
    namespace
    {
        /** 割合の範囲 */
        constexpr float MIN_RATE = 0.0f;
        constexpr float MAX_RATE = 1.0f;
    } // namespace


    VolumeFader::VolumeFader(const float initialRate)
        : m_current(std::clamp(initialRate, MIN_RATE, MAX_RATE))
        , m_target(m_current)
        , m_speed(0.0f)
    {
    }


    void VolumeFader::FadeTo(const float targetRate, const float seconds)
    {
        m_target = std::clamp(targetRate, MIN_RATE, MAX_RATE);

        if (seconds <= 0.0f)
        {
            m_current = m_target;
            m_speed = 0.0f;
            return;
        }
        m_speed = std::abs(m_target - m_current) / seconds;
    }


    void VolumeFader::Update(const float deltaTime)
    {
        if (!IsFading())
        {
            return;
        }

        // NOTE: Windowsのmin/maxマクロと衝突するので、std::min / std::maxは使わない。
        const float step = m_speed * deltaTime;
        if (m_current < m_target)
        {
            m_current += step;
            if (m_current > m_target)
            {
                m_current = m_target;
            }
        }
        else
        {
            m_current -= step;
            if (m_current < m_target)
            {
                m_current = m_target;
            }
        }
    }
} // namespace app
