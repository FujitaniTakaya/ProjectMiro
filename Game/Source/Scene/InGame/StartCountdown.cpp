/**
 * @file StartCountdown.cpp
 * @brief ゲーム開始前のカウントダウンの時間管理
 */
#include "stdafx.h"

#include "StartCountdown.h"


namespace app
{
    StartCountdown::StartCountdown(const float startDelay, const int32_t count, const float interval)
        : m_startDelay(startDelay > 0.0f ? startDelay : 0.0f)
        , m_count(count > 0 ? count : 0)
        , m_interval(interval > 0.0f ? interval : 0.0f)
        , m_timer()
    {
        m_timer.SetTimeLimit(m_startDelay + static_cast<float>(m_count) * m_interval);
    }


    void StartCountdown::Update(const float deltaTime)
    {
        // 制限時間を超えては進まない。
        m_timer.Update(deltaTime);
    }


    bool StartCountdown::IsCounting() const
    {
        return m_timer.GetElapsedTime() >= m_startDelay && !IsFinished();
    }


    int32_t StartCountdown::GetNumber() const
    {
        // 数字を出している間は、index が 0 〜 count-1 になる。
        const int32_t index = static_cast<int32_t>((m_timer.GetElapsedTime() - m_startDelay) / m_interval);
        return m_count - index;
    }


    float StartCountdown::GetNumberElapsed() const
    {
        const float elapsed = m_timer.GetElapsedTime() - m_startDelay;
        const float index = static_cast<float>(static_cast<int32_t>(elapsed / m_interval));
        return elapsed - index * m_interval;
    }
} // namespace app
