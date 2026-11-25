/**
 * @file ResultMenu.cpp
 * @brief リザルト画面のUIを動かすMenu
 */
#include "stdafx.h"

#include "ResultMenu.h"

#include <string>

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
            /** 内訳の行の数 */
            constexpr int ROW_NUM = static_cast<int>(EnResultRow::Max);
            /** 項目の文字のUIの名前(ハッシュ値)。EnResultItem の順 */
            constexpr uint32_t ITEM_KEYS[ITEM_NUM] = {
                Hash32("ItemTitle"),
                Hash32("ItemRetry"),
            };
            /** 選択カーソルの矢印のUIの名前(ハッシュ値) */
            constexpr uint32_t CURSOR_KEY = Hash32("Cursor");
            /** 内訳の行の数字の文字のUIの名前(ハッシュ値)。EnResultRow の順 */
            constexpr uint32_t VALUE_KEYS[ROW_NUM] = {
                Hash32("TimeValue"),
                Hash32("NormalValue"),
                Hash32("RunningValue"),
            };
            /** 内訳の行の点の文字のUIの名前(ハッシュ値)。EnResultRow の順 */
            constexpr uint32_t POINTS_KEYS[ROW_NUM] = {
                Hash32("TimePoints"),
                Hash32("NormalPoints"),
                Hash32("RunningPoints"),
            };
            /** 合計(スコア)の文字のUIの名前(ハッシュ値) */
            constexpr uint32_t TOTAL_KEY = Hash32("TotalPoints");
            /** 1分の秒数 */
            constexpr int SECONDS_PER_MINUTE = 60;
            /** 秒を2桁に揃える時の、1桁になる境目 */
            constexpr int TWO_DIGIT_SECONDS = 10;


            /**
             * @brief 守衛時間を「m:ss」の文字列にする
             * @details 秒は切り捨てる。(インゲームの経過時間の表示・スコアの点と同じ)
             * @param seconds 守衛継続時間(秒)。負の値とNaNは0として扱う
             * @return 「m:ss」の文字列
             */
            std::wstring FormatTime(const float seconds)
            {
                const int total = seconds > 0.0f ? static_cast<int>(seconds) : 0;
                const int sec = total % SECONDS_PER_MINUTE;
                std::wstring text = std::to_wstring(total / SECONDS_PER_MINUTE) + L":";
                if (sec < TWO_DIGIT_SECONDS)
                {
                    text += L"0";
                }
                return text + std::to_wstring(sec);
            }


            /**
             * @brief 文字に、文字列を書く(文字が無ければ何もしない)
             * @param text 書き込み先の文字。nullptr なら何もしない
             * @param value 書く文字列
             */
            void SetText(UIText* text, const std::wstring& value)
            {
                if (text)
                {
                    text->SetText(value.c_str());
                }
            }
        } // namespace


        ResultMenu::ResultMenu()
            : m_items()
            , m_cursor(nullptr)
            , m_tint()
            , m_selectedItem(EnResultItem::Title)
            , m_valueTexts()
            , m_pointsTexts()
            , m_totalText(nullptr)
            , m_data()
            , m_score()
            , m_hasResult(false)
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

            for (int i = 0; i < ROW_NUM; ++i)
            {
                m_valueTexts[i] = GetUI<UIText>(VALUE_KEYS[i]);
                m_pointsTexts[i] = GetUI<UIText>(POINTS_KEYS[i]);
                if (!m_valueTexts[i] || !m_pointsTexts[i])
                {
                    K2_LOG("ResultMenu: UIText の内訳の行 %d の数字か点がありません。\n", i);
                }
            }
            m_totalText = GetUI<UIText>(TOTAL_KEY);
            if (!m_totalText)
            {
                K2_LOG("ResultMenu: UIText \"TotalPoints\" がありません。\n");
            }
            // パーツが作り直されて、文字がjsonの値に戻ったので、結果を書き直す。
            ApplyResult();
        }


        void ResultMenu::SetResult(const GameResultData& data, const ScoreBreakdown& score)
        {
            m_data = data;
            m_score = score;
            m_hasResult = true;
            ApplyResult();
        }


        void ResultMenu::ApplyResult()
        {
            if (!m_hasResult)
            {
                return;
            }

            const std::wstring points = L"点";
            const std::wstring kills = L"体";
            SetText(m_valueTexts[static_cast<size_t>(EnResultRow::Time)], FormatTime(m_data.survivedSeconds));
            SetText(m_valueTexts[static_cast<size_t>(EnResultRow::Normal)], std::to_wstring(m_data.normalKills) + kills);
            SetText(m_valueTexts[static_cast<size_t>(EnResultRow::Running)], std::to_wstring(m_data.runningKills) + kills);
            SetText(m_pointsTexts[static_cast<size_t>(EnResultRow::Time)], std::to_wstring(m_score.timePoints) + points);
            SetText(m_pointsTexts[static_cast<size_t>(EnResultRow::Normal)], std::to_wstring(m_score.normalPoints) + points);
            SetText(m_pointsTexts[static_cast<size_t>(EnResultRow::Running)], std::to_wstring(m_score.runningPoints) + points);
            SetText(m_totalText, std::to_wstring(m_score.total) + points);
        }


        void ResultMenu::MoveSelection(const int delta)
        {
            const int index = (static_cast<int>(m_selectedItem) + delta) % ITEM_NUM;
            m_selectedItem = static_cast<EnResultItem>((index + ITEM_NUM) % ITEM_NUM);
        }
    } // namespace ui
} // namespace app
