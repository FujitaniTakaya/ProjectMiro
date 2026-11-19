/**
 * @file ResultMenu.cpp
 * @brief リザルト画面のUIを動かすMenu
 */
#include "stdafx.h"

#include "ResultMenu.h"

#include "Source/Core/Util/CRC32.h"
#include "Source/UI/Common/CursorSway.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 項目の数 */
            constexpr int ITEM_NUM = static_cast<int>(EnResultItem::Max);
            /** 項目の文字のUIの名前(ハッシュ値)。EnResultItem の順 */
            constexpr uint32_t ITEM_KEYS[ITEM_NUM] = {
                Hash32("ItemTitle"),
                Hash32("ItemQuit"),
            };
            /** 選択カーソルの矢印のUIの名前(ハッシュ値) */
            constexpr uint32_t CURSOR_KEY = Hash32("Cursor");
        } // namespace


        ResultMenu::ResultMenu()
            : m_items()
            , m_cursor(nullptr)
            , m_tint()
            , m_selectedItem(EnResultItem::Title)
        {}


        ResultMenu::~ResultMenu() = default;


        void ResultMenu::Update()
        {
            // NOTE: Layoutがjsonの更新でUIのパーツを作り直すと、座標がjsonの値に戻る。毎フレーム書いて、すぐ直す。
            const UIText* selected = m_items[static_cast<size_t>(m_selectedItem)];
            if (m_cursor && selected)
            {
                m_cursor->m_transform.m_localTransform.m_position.y = selected->m_transform.m_localTransform.m_position.y;
            }

            // NOTE: 色は、MenuBase::Update() が描画に渡す前に書く。
            m_tint.Step(static_cast<int>(m_selectedItem), g_gameTime->GetFrameDeltaTime());

            MenuBase::Update();
        }


        void ResultMenu::InitializeLogic()
        {
            m_tint.Clear();
            for (int i = 0; i < ITEM_NUM; ++i)
            {
                m_items[i] = GetUI<UIText>(ITEM_KEYS[i]);
                if (!m_items[i])
                {
                    K2_LOG("ResultMenu: UIText の項目 %d がありません。\n", i);
                }
                m_tint.AddPart(i, m_items[i]);
            }
            // パーツが作り直されて、色がjsonの値に戻ったので、すぐ書き直す。
            m_tint.Snap(static_cast<int>(m_selectedItem));

            m_cursor = GetUI<UIText>(CURSOR_KEY);
            if (!m_cursor)
            {
                K2_LOG("ResultMenu: UIText \"Cursor\" がありません。\n");
            }
            AttachCursorSway(m_cursor);
        }


        void ResultMenu::MoveSelection(const int delta)
        {
            const int index = (static_cast<int>(m_selectedItem) + delta) % ITEM_NUM;
            m_selectedItem = static_cast<EnResultItem>((index + ITEM_NUM) % ITEM_NUM);
        }
    } // namespace ui
} // namespace app
