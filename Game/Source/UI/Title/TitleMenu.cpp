/**
 * @file TitleMenu.cpp
 * @brief タイトル画面のUIを動かすMenu
 */
#include "stdafx.h"

#include "TitleMenu.h"

#include <cmath>
#include <cstdint>

#include "Source/Core/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 「PRESS START」の文字のUIの名前(ハッシュ値) */
            constexpr uint32_t PRESS_START_KEY = Hash32("PressStart");
            /** 点滅の周期(秒) */
            constexpr float BLINK_PERIOD = 1.6f;
            /** 点滅で一番薄くなった時の透明度(0〜1) */
            constexpr float BLINK_MIN_ALPHA = 0.15f;
        } // namespace


        TitleMenu::TitleMenu()
            : m_pressStart(nullptr)
            , m_blinkTimer(0.0f)
        {}


        TitleMenu::~TitleMenu() = default;


        void TitleMenu::Update()
        {
            m_blinkTimer = std::fmod(m_blinkTimer + g_gameTime->GetFrameDeltaTime(), BLINK_PERIOD);

            // 周期の頭で一番濃く、半分で一番薄くなる。
            // NOTE: Layoutがjsonの更新でUIのパーツを作り直すと、色がjsonの値に戻る。毎フレーム書いて、すぐ直す。
            if (m_pressStart)
            {
                const float wave = 0.5f + 0.5f * std::cos(m_blinkTimer / BLINK_PERIOD * Math::PI2);
                m_pressStart->m_color.w = BLINK_MIN_ALPHA + (1.0f - BLINK_MIN_ALPHA) * wave;
            }

            MenuBase::Update();
        }


        void TitleMenu::InitializeLogic()
        {
            m_pressStart = GetUI<UIText>(PRESS_START_KEY);
            if (!m_pressStart)
            {
                K2_LOG("TitleMenu: UIText \"PressStart\" がありません。\n");
            }
        }
    } // namespace ui
} // namespace app
