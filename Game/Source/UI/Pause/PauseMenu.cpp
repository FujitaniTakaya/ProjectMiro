/**
 * @file PauseMenu.cpp
 * @brief ポーズ画面のUIを動かすMenu
 */
#include "stdafx.h"

#include "PauseMenu.h"

#include <algorithm>

#include "Source/Core/Util/CRC32.h"
#include "Source/UI/CursorSway.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 項目の数 */
            constexpr int ITEM_NUM = static_cast<int>(EnPauseItem::Max);
            /** 項目の文字のUIの名前(ハッシュ値)。EnPauseItem の順 */
            constexpr uint32_t ITEM_KEYS[ITEM_NUM] = {
                Hash32("ItemOption"),
                Hash32("ItemTitle"),
            };
            /** 選択カーソルの矢印のUIの名前(ハッシュ値) */
            constexpr uint32_t CURSOR_KEY = Hash32("Cursor");
            /** 背景(画面を暗くする板)のUIの名前(ハッシュ値) */
            constexpr uint32_t BACKDROP_KEY = Hash32("Backdrop");
            /** 見出しのUIの名前(ハッシュ値) */
            constexpr uint32_t HEADING_KEY = Hash32("Heading");
        } // namespace


        PauseMenu::PauseMenu()
            : m_items()
            , m_cursor(nullptr)
            , m_tint()
            , m_fadeParts()
            , m_selectedItem(EnPauseItem::Option)
            , m_alpha(0.0f)
        {}


        PauseMenu::~PauseMenu() = default;


        void PauseMenu::Update()
        {
            m_tint.Step(static_cast<int>(m_selectedItem), g_gameTime->GetFrameDeltaTime());
            ApplyView();

            MenuBase::Update();
        }


        void PauseMenu::InitializeLogic()
        {
            m_fadeParts.clear();
            m_tint.Clear();

            for (int i = 0; i < ITEM_NUM; ++i)
            {
                m_items[i] = GetUI<UIText>(ITEM_KEYS[i]);
                if (!m_items[i])
                {
                    K2_LOG("PauseMenu: UIText の項目 %d がありません。\n", i);
                }
                AddFadePart(m_items[i]);
                m_tint.AddPart(i, m_items[i]);
            }

            m_cursor = GetUI<UIText>(CURSOR_KEY);
            if (!m_cursor)
            {
                K2_LOG("PauseMenu: UIText \"Cursor\" がありません。\n");
            }
            AddFadePart(m_cursor);
            AttachCursorSway(m_cursor);

            UIBase* backdrop = GetUI<UIBase>(BACKDROP_KEY);
            if (!backdrop)
            {
                K2_LOG("PauseMenu: UI \"Backdrop\" がありません。\n");
            }
            AddFadePart(backdrop);

            UIBase* heading = GetUI<UIBase>(HEADING_KEY);
            if (!heading)
            {
                K2_LOG("PauseMenu: UI \"Heading\" がありません。\n");
            }
            AddFadePart(heading);

            // パーツが作り直されて、位置・色・透明度がjsonの値に戻ったので、書き直す。
            m_tint.Snap(static_cast<int>(m_selectedItem));
            ApplyView();
        }


        void PauseMenu::Open()
        {
            m_selectedItem = EnPauseItem::Option;
            m_tint.Snap(static_cast<int>(m_selectedItem));
            ApplyView();
        }


        void PauseMenu::MoveSelection(const int delta)
        {
            const int index = (static_cast<int>(m_selectedItem) + delta) % ITEM_NUM;
            m_selectedItem = static_cast<EnPauseItem>((index + ITEM_NUM) % ITEM_NUM);
        }


        void PauseMenu::SetAlpha(const float alpha)
        {
            m_alpha = std::clamp(alpha, 0.0f, 1.0f);
            ApplyView();
        }


        void PauseMenu::AddFadePart(UIBase* ui)
        {
            if (ui)
            {
                m_fadeParts.push_back({ ui, ui->m_color.w });
            }
        }


        void PauseMenu::ApplyView()
        {
            // NOTE: Layoutがjsonの更新でUIのパーツを作り直すと、座標と透明度がjsonの値に戻る。毎フレーム書いて、すぐ直す。
            const UIText* selected = m_items[static_cast<size_t>(m_selectedItem)];
            if (m_cursor && selected)
            {
                m_cursor->m_transform.m_localTransform.m_position.y = selected->m_transform.m_localTransform.m_position.y;
            }

            for (const FadePart& part : m_fadeParts)
            {
                part.m_ui->m_color.w = part.m_baseAlpha * m_alpha;
            }
        }
    } // namespace ui
} // namespace app
