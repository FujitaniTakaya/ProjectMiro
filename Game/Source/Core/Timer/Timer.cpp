/**
 * @file Timer.cpp
 * @brief 制限時間を数えるクラス
 */
#include "stdafx.h"

#include "Timer.h"


namespace app
{
    Timer::Timer()
        : m_timeLimit(0.0f)
        , m_elapsed(0.0f)
        , m_handle(INVALID_TIMER_HANDLE)
    {
    }


    void Timer::SetTimeLimit(const float seconds)
    {
        // NaNも0にするため、「0より大きい」の否定で判定する。
        m_timeLimit = seconds > 0.0f ? seconds : 0.0f;
        m_elapsed = 0.0f;
    }


    void Timer::Update(const float deltaTime)
    {
        // NaNも弾くため、「0より大きい」の否定で判定する。
        if (!(deltaTime > 0.0f))
        {
            return;
        }

        // NOTE: Windowsのmin/maxマクロと衝突するので、std::minは使わない。
        m_elapsed += deltaTime;
        if (m_elapsed > m_timeLimit)
        {
            m_elapsed = m_timeLimit;
        }
    }


    void Timer::Reset()
    {
        m_elapsed = 0.0f;
    }


    float Timer::GetProgress() const
    {
        // 制限時間が0のタイマーは、0で割らずに時間切れとして扱う。
        if (m_timeLimit <= 0.0f)
        {
            return 1.0f;
        }
        return m_elapsed / m_timeLimit;
    }
} // namespace app
