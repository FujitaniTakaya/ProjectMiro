/**
 * @file GameSetMenu.cpp
 * @brief ゲームセットの文字を動かすMenu
 */
#include "stdafx.h"

#include "GameSetMenu.h"

#include <cstdint>

#include "Source/Core/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 「GAME SET」の文字のUIの名前(ハッシュ値) */
            constexpr uint32_t TEXT_KEY = Hash32("GameSetText");

            /** 出た瞬間の大きさの倍率(1.0が、jsonで決めた大きさ) */
            constexpr float POP_SCALE = 1.5f;
            /** 出た瞬間の大きさから、元の大きさに戻るまでの時間(秒) */
            constexpr float POP_TIME = 0.4f;
        } // namespace


        GameSetMenu::GameSetMenu()
            : m_text(nullptr)
            , m_isShowing(false)
            , m_elapsed(0.0f)
        {}


        GameSetMenu::~GameSetMenu() = default;


        void GameSetMenu::InitializeLogic()
        {
            m_text = GetUI<UIText>(TEXT_KEY);
            if (!m_text)
            {
                K2_LOG("GameSetMenu: UIText \"GameSetText\" がありません。\n");
                return;
            }

            // ホットリロードで作り直された時も、出している間は出したままにする
            m_text->SetIsDraw(m_isShowing);
            if (m_isShowing)
            {
                ApplyPop();
            }
        }


        void GameSetMenu::Update()
        {
            // 縮み終わったら、時間は進めない
            if (m_isShowing && m_elapsed < POP_TIME)
            {
                m_elapsed += g_gameTime->GetFrameDeltaTime();
                ApplyPop();
            }

            MenuBase::Update();
        }


        void GameSetMenu::Show()
        {
            if (!m_text || m_isShowing)
            {
                return;
            }

            m_isShowing = true;
            m_elapsed = 0.0f;
            ApplyPop();
            m_text->SetIsDraw(true);
        }


        void GameSetMenu::ApplyPop()
        {
            // 出た瞬間は POP_SCALE 倍で、POP_TIME 秒かけて1倍へ縮む。
            // NOTE: Windowsのmin/maxマクロと衝突するので、std::minは使わない。
            float t = m_elapsed / POP_TIME;
            if (t > 1.0f)
            {
                t = 1.0f;
            }
            const float scale = POP_SCALE + (1.0f - POP_SCALE) * t;
            m_text->m_transform.m_localTransform.m_scale = Vector3(scale, scale, 1.0f);
        }
    } // namespace ui
} // namespace app
