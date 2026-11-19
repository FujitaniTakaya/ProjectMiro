/**
 * @file GameTimerMenu.cpp
 * @brief インゲームの経過時間を動かすMenu
 */
#include "stdafx.h"

#include "GameTimerMenu.h"

#include <cstdint>
#include <string>

#include "Source/Core/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 経過時間の文字のUIの名前(ハッシュ値) */
            constexpr uint32_t TIMER_TEXT_KEY = Hash32("TimerText");
            /** 1分の秒数 */
            constexpr int32_t SECONDS_PER_MINUTE = 60;
            /** 秒を2桁に揃える時の、1桁になる境目 */
            constexpr int32_t TWO_DIGIT_SECONDS = 10;
        } // namespace


        GameTimerMenu::GameTimerMenu()
            : m_timerText(nullptr)
            , m_displaySeconds(-1)
        {}


        GameTimerMenu::~GameTimerMenu() = default;


        void GameTimerMenu::InitializeLogic()
        {
            m_timerText = GetUI<UIText>(TIMER_TEXT_KEY);
            if (!m_timerText)
            {
                K2_LOG("GameTimerMenu: UIText \"TimerText\" がありません。\n");
            }

            ApplyTime();
        }


        void GameTimerMenu::SetElapsedTime(const float seconds)
        {
            // NaNも0にするため、「0より大きい」の否定で判定する。正の値のintへのキャストは切り捨て
            const int32_t displaySeconds = seconds > 0.0f ? static_cast<int32_t>(seconds) : 0;
            if (displaySeconds == m_displaySeconds)
            {
                return;
            }

            m_displaySeconds = displaySeconds;
            ApplyTime();
        }


        void GameTimerMenu::ApplyTime()
        {
            if (!m_timerText || m_displaySeconds < 0)
            {
                return;
            }

            // 「m:ss」。秒は2桁に揃える
            const int32_t minutes = m_displaySeconds / SECONDS_PER_MINUTE;
            const int32_t seconds = m_displaySeconds % SECONDS_PER_MINUTE;
            std::wstring text = std::to_wstring(minutes) + L":";
            if (seconds < TWO_DIGIT_SECONDS)
            {
                text += L"0";
            }
            text += std::to_wstring(seconds);
            m_timerText->SetText(text.c_str());
        }
    } // namespace ui
} // namespace app
