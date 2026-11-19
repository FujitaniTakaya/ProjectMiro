/**
 * @file CountdownMenu.cpp
 * @brief ゲーム開始前のカウントダウンの数字を動かすMenu
 */
#include "stdafx.h"

#include "CountdownMenu.h"

#include <cstdint>
#include <string>

#include "Source/Core/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** カウントダウンの数字のUIの名前(ハッシュ値) */
            constexpr uint32_t NUMBER_KEY = Hash32("CountdownNumber");

            /** 数字が出た瞬間の大きさの倍率(1.0が、jsonで決めた大きさ) */
            constexpr float POP_SCALE = 1.5f;
            /** 出た瞬間の大きさから、元の大きさに戻るまでの時間(秒) */
            constexpr float POP_TIME = 0.25f;

            /** 「Start」の文字列 */
            constexpr const wchar_t* START_TEXT = L"Start";
            /** 「Start」の元の大きさの倍率。数字より文字数が多いので、小さくして画面からはみ出さないようにする */
            constexpr float START_SCALE = 0.6f;
            /** 「Start」を出している時間(秒)。この時間が経つと消える */
            constexpr float START_DISPLAY_TIME = 1.0f;
        } // namespace


        CountdownMenu::CountdownMenu()
            : m_number(nullptr)
            , m_isShowingStart(false)
            , m_startElapsed(0.0f)
        {}


        CountdownMenu::~CountdownMenu() = default;


        void CountdownMenu::InitializeLogic()
        {
            m_number = GetUI<UIText>(NUMBER_KEY);
            if (!m_number)
            {
                K2_LOG("CountdownMenu: UIText \"CountdownNumber\" がありません。\n");
            }

            Hide();
        }


        void CountdownMenu::Update()
        {
            // 「Start」を出している間は、時間を進めて、一定の時間が経ったら消す
            if (m_isShowingStart)
            {
                m_startElapsed += g_gameTime->GetFrameDeltaTime();
                if (m_startElapsed >= START_DISPLAY_TIME)
                {
                    Hide();
                }
                else
                {
                    ApplyPop(START_SCALE, m_startElapsed);
                }
            }

            MenuBase::Update();
        }


        void CountdownMenu::Show(const int32_t number, const float elapsed)
        {
            if (!m_number)
            {
                return;
            }

            m_isShowingStart = false;
            m_number->SetText(std::to_wstring(number).c_str());
            ApplyPop(1.0f, elapsed);
            m_number->SetIsDraw(true);
        }


        void CountdownMenu::ShowStart()
        {
            if (!m_number)
            {
                return;
            }

            m_isShowingStart = true;
            m_startElapsed = 0.0f;
            m_number->SetText(START_TEXT);
            ApplyPop(START_SCALE, 0.0f);
            m_number->SetIsDraw(true);
        }


        void CountdownMenu::Hide()
        {
            m_isShowingStart = false;
            if (m_number)
            {
                m_number->SetIsDraw(false);
            }
        }


        void CountdownMenu::ApplyPop(const float baseScale, const float elapsed)
        {
            if (!m_number)
            {
                return;
            }

            // 出た瞬間は POP_SCALE 倍で、POP_TIME 秒かけて1倍へ縮む。
            // NOTE: Windowsのmin/maxマクロと衝突するので、std::minは使わない。
            float t = elapsed / POP_TIME;
            if (t > 1.0f)
            {
                t = 1.0f;
            }
            const float scale = baseScale * (POP_SCALE + (1.0f - POP_SCALE) * t);
            m_number->m_transform.m_localTransform.m_scale = Vector3(scale, scale, 1.0f);
        }
    } // namespace ui
} // namespace app
