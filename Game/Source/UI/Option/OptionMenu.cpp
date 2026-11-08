/**
 * @file OptionMenu.cpp
 * @brief オプション画面のUIを動かすMenu
 */
#include "stdafx.h"

#include "OptionMenu.h"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <iterator>

#include "Source/Core/Util/CRC32.h"
#include "Source/Option/OptionSettings.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/CursorSway.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 項目の数 */
            constexpr int ITEM_NUM = static_cast<int>(EnOptionItem::Max);
            /** 項目の文字のUIの名前(ハッシュ値)。EnOptionItem の順 */
            constexpr uint32_t ITEM_KEYS[ITEM_NUM] = {
                Hash32("ItemSound"),
                Hash32("ItemSensitivity"),
            };
            /** 項目を選ぶ矢印のUIの名前(ハッシュ値) */
            constexpr uint32_t CURSOR_KEY = Hash32("Cursor");
            /** 設定画面の板のUIの名前(ハッシュ値) */
            constexpr uint32_t PANEL_KEY = Hash32("Panel");
            /** 設定画面の見出しのUIの名前(ハッシュ値) */
            constexpr uint32_t PANEL_TITLE_KEY = Hash32("PanelTitle");
            /** 設定画面の行を選ぶ矢印のUIの名前(ハッシュ値) */
            constexpr uint32_t ROW_CURSOR_KEY = Hash32("RowCursor");
            /** 設定画面の行のラベルのUIの名前(ハッシュ値) */
            constexpr uint32_t ROW_LABEL_KEYS[OptionMenu::ROW_SLOT_NUM] = {
                Hash32("Row0Label"),
                Hash32("Row1Label"),
                Hash32("Row2Label"),
            };
            /** 設定画面の行のバーの背景のUIの名前(ハッシュ値) */
            constexpr uint32_t ROW_BAR_BG_KEYS[OptionMenu::ROW_SLOT_NUM] = {
                Hash32("Row0BarBg"),
                Hash32("Row1BarBg"),
                Hash32("Row2BarBg"),
            };
            /** 設定画面の行のバーのUIの名前(ハッシュ値) */
            constexpr uint32_t ROW_BAR_FILL_KEYS[OptionMenu::ROW_SLOT_NUM] = {
                Hash32("Row0BarFill"),
                Hash32("Row1BarFill"),
                Hash32("Row2BarFill"),
            };
            /** 設定画面の行の値の文字のUIの名前(ハッシュ値) */
            constexpr uint32_t ROW_VALUE_KEYS[OptionMenu::ROW_SLOT_NUM] = {
                Hash32("Row0Value"),
                Hash32("Row1Value"),
                Hash32("Row2Value"),
            };

            /** 入力を受け付けるパッドの番号 */
            constexpr int PAD_INDEX = 0;
            /** 設定画面が開く・閉じるのにかける時間(秒) */
            constexpr float PANEL_FADE_TIME = 0.15f;
            /** 音量の刻み(0〜1) */
            constexpr float VOLUME_STEP = 0.05f;
            /** 文字に書いた値が、まだ無い印。どの値とも一致しない */
            constexpr float NO_SHOWN_VALUE = -1.0f;
            /** 値の文字の長さ */
            constexpr size_t VALUE_TEXT_SIZE = 16;


            /**
             * @brief 設定画面で動かす値の種類
             */
            enum class EnOptionValue : uint8_t
            {
                MasterVolume,      //!< 全体の音量
                BgmVolume,         //!< BGMの音量
                SeVolume,          //!< SEの音量
                CameraSensitivity, //!< カメラ感度
                AimSensitivity,    //!< エイム感度
            };


            /**
             * @brief 設定画面の1行
             */
            struct PageRow
            {
                /** 行の番号(jsonの Row0〜Row2) */
                int m_slot;
                /** ラベル */
                const wchar_t* m_label;
                /** 動かす値 */
                EnOptionValue m_value;
            };


            /**
             * @brief 設定画面の1ページ
             */
            struct Page
            {
                /** 行 */
                const PageRow* m_rows;
                /** 行の数 */
                int m_rowNum;
            };


            /** サウンドの設定画面の行 */
            constexpr PageRow SOUND_ROWS[] = {
                { 0, L"全体", EnOptionValue::MasterVolume },
                { 1, L"BGM", EnOptionValue::BgmVolume },
                { 2, L"SE", EnOptionValue::SeVolume },
            };
            /** 感度の設定画面の行。カメラ感度とエイム感度(ハンドガンを構えている時の感度) */
            constexpr PageRow SENSITIVITY_ROWS[] = {
                { 0, L"カメラ", EnOptionValue::CameraSensitivity },
                { 1, L"エイム", EnOptionValue::AimSensitivity },
            };
            /** 項目ごとの設定画面。EnOptionItem の順 */
            constexpr Page PAGES[ITEM_NUM] = {
                { SOUND_ROWS, static_cast<int>(std::size(SOUND_ROWS)) },
                { SENSITIVITY_ROWS, static_cast<int>(std::size(SENSITIVITY_ROWS)) },
            };


            /**
             * @brief 値が音量なら、その音量を取得する
             * @param value 値の種類
             * @return 音量。音量でない場合はnullptr
             */
            SoundVolume* FindVolume(const EnOptionValue value)
            {
                SoundManager& sound = SoundManager::Get();
                switch (value)
                {
                case EnOptionValue::MasterVolume:
                    return &sound.GetMaster();
                case EnOptionValue::BgmVolume:
                    return &sound.GetBGM();
                case EnOptionValue::SeVolume:
                    return &sound.GetSE();
                default:
                    return nullptr;
                }
            }


            /**
             * @brief 値を取得する
             * @param value 値の種類
             * @return 値。音量は0〜1、感度は倍率
             */
            float GetValue(const EnOptionValue value)
            {
                if (const SoundVolume* volume = FindVolume(value))
                {
                    return volume->GetVolume();
                }

                const OptionSettings& settings = OptionSettings::Get();
                return (value == EnOptionValue::CameraSensitivity) ? settings.GetCameraSensitivity() : settings.GetAimSensitivity();
            }


            /**
             * @brief 値を1段階動かす
             * @param value 値の種類
             * @param direction -1で下げる、+1で上げる
             */
            void StepValue(const EnOptionValue value, const int direction)
            {
                if (SoundVolume* volume = FindVolume(value))
                {
                    // 刻みの倍数に揃えてから動かす。範囲外の丸めは、SoundVolume側が行う。
                    const float steps = std::round(volume->GetVolume() / VOLUME_STEP) + static_cast<float>(direction);
                    volume->SetVolume(steps * VOLUME_STEP);
                    return;
                }

                OptionSettings& settings = OptionSettings::Get();
                const float delta = static_cast<float>(direction) * OptionSettings::SENSITIVITY_STEP;
                if (value == EnOptionValue::CameraSensitivity)
                {
                    settings.SetCameraSensitivity(settings.GetCameraSensitivity() + delta);
                }
                else
                {
                    settings.SetAimSensitivity(settings.GetAimSensitivity() + delta);
                }
            }


            /**
             * @brief 値を、バーの割合(0〜1)にする
             * @param value 値の種類
             * @param current 値
             * @return バーの割合。感度は、下限が0、上限が1
             */
            float ToRatio(const EnOptionValue value, const float current)
            {
                if (FindVolume(value) != nullptr)
                {
                    return std::clamp(current, 0.0f, 1.0f);
                }

                const float ratio = (current - OptionSettings::MIN_SENSITIVITY) / (OptionSettings::MAX_SENSITIVITY - OptionSettings::MIN_SENSITIVITY);
                return std::clamp(ratio, 0.0f, 1.0f);
            }


            /**
             * @brief 値を、画面に出す文字にする
             * @param value 値の種類
             * @param current 値
             * @param[out] buffer 文字の書き込み先
             * @param size 書き込み先の長さ
             */
            void FormatValue(const EnOptionValue value, const float current, wchar_t* buffer, const size_t size)
            {
                if (FindVolume(value) != nullptr)
                {
                    // 音量は0〜100で出す
                    swprintf_s(buffer, size, L"%d", static_cast<int>(std::round(current * 100.0f)));
                }
                else
                {
                    swprintf_s(buffer, size, L"%.1f", current);
                }
            }
        } // namespace


        OptionMenu::OptionMenu()
            : m_items()
            , m_cursor(nullptr)
            , m_panelTitle(nullptr)
            , m_rowCursor(nullptr)
            , m_rows()
            , m_itemTint()
            , m_rowTint()
            , m_panelParts()
            , m_shownValues()
            , m_input()
            , m_selectedItem(EnOptionItem::Sound)
            , m_selectedRow(0)
            , m_panelAlpha(0.0f)
            , m_isPanelOpen(false)
            , m_isClosed(false)
            , m_isDirty(true)
        {
            m_shownValues.fill(NO_SHOWN_VALUE);
        }


        OptionMenu::~OptionMenu() = default;


        template <typename T>
        T* OptionMenu::FindPanelPart(const uint32_t key, const int slot)
        {
            T* ui = GetUI<T>(key);
            if (!ui)
            {
                K2_LOG("OptionMenu: 設定画面のUIがありません。key=%u\n", key);
                return nullptr;
            }

            m_panelParts.push_back({ ui, ui->m_color.w, slot });
            return ui;
        }


        void OptionMenu::Update()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();

            if (const GamePad* pad = MenuInput::FindPad(PAD_INDEX))
            {
                m_input.Update(*pad, deltaTime);
                if (!m_isClosed)
                {
                    HandleInput(*pad);
                }
            }

            UpdatePanelAlpha(deltaTime);
            m_itemTint.Step(static_cast<int>(m_selectedItem), deltaTime);
            m_rowTint.Step(GetSelectedSlot(), deltaTime);
            ApplyView();

            MenuBase::Update();
        }


        void OptionMenu::InitializeLogic()
        {
            m_itemTint.Clear();
            m_rowTint.Clear();

            for (int i = 0; i < ITEM_NUM; ++i)
            {
                m_items[i] = GetUI<UIText>(ITEM_KEYS[i]);
                if (!m_items[i])
                {
                    K2_LOG("OptionMenu: UIText の項目 %d がありません。\n", i);
                }
                m_itemTint.AddPart(i, m_items[i]);
            }

            m_cursor = GetUI<UIText>(CURSOR_KEY);
            if (!m_cursor)
            {
                K2_LOG("OptionMenu: UIText \"Cursor\" がありません。\n");
            }
            AttachCursorSway(m_cursor);

            // 設定画面のパーツ。jsonの透明度を覚えて、開閉のフェードで掛け直す。
            m_panelParts.clear();
            FindPanelPart<UIBase>(PANEL_KEY, -1);
            m_panelTitle = FindPanelPart<UIText>(PANEL_TITLE_KEY, -1);
            m_rowCursor = FindPanelPart<UIText>(ROW_CURSOR_KEY, -1);
            AttachCursorSway(m_rowCursor);
            for (int slot = 0; slot < ROW_SLOT_NUM; ++slot)
            {
                m_rows[slot].m_label = FindPanelPart<UIText>(ROW_LABEL_KEYS[slot], slot);
                FindPanelPart<UIBase>(ROW_BAR_BG_KEYS[slot], slot);
                m_rows[slot].m_fill = FindPanelPart<UIGauge>(ROW_BAR_FILL_KEYS[slot], slot);
                m_rows[slot].m_value = FindPanelPart<UIText>(ROW_VALUE_KEYS[slot], slot);

                // 薄灰色にするのは、行の文字とバーの中身。バーの背景(溝)は変えない。
                m_rowTint.AddPart(slot, m_rows[slot].m_label);
                m_rowTint.AddPart(slot, m_rows[slot].m_fill);
                m_rowTint.AddPart(slot, m_rows[slot].m_value);
            }

            // パーツが作り直されて、文字と色がjsonの値に戻ったので、書き直す。
            m_itemTint.Snap(static_cast<int>(m_selectedItem));
            m_rowTint.Snap(GetSelectedSlot());
            m_shownValues.fill(NO_SHOWN_VALUE);
            m_isDirty = true;

            // 作り直されたパーツは、jsonの値(全て描く状態)になっている。次のUpdate()を待つと、その間に描かれてしまうので、すぐ直す。
            ApplyView();
        }


        void OptionMenu::Open()
        {
            m_selectedItem = EnOptionItem::Sound;
            m_selectedRow = 0;
            m_isPanelOpen = false;
            m_panelAlpha = 0.0f;
            m_isClosed = false;
            m_input.Reset();
            m_isDirty = true;
            m_itemTint.Snap(static_cast<int>(m_selectedItem));
            m_rowTint.Snap(GetSelectedSlot());

            // 決めたフレームのうちに、このMenuも描かれる。次のUpdate()を待つと、前回の見た目が1フレーム映るので、すぐ直す。
            ApplyView();
        }


        void OptionMenu::HandleInput(const GamePad& pad)
        {
            const int moveX = m_input.GetMoveX();
            const int moveY = m_input.GetMoveY();

            if (!m_isPanelOpen)
            {
                if (moveY != 0)
                {
                    MoveItem(moveY);
                }

                if (pad.IsTrigger(enButtonA) || pad.IsTrigger(enButtonStart))
                {
                    OpenPanel();
                }
                else if (pad.IsTrigger(enButtonB))
                {
                    m_isClosed = true;
                }
                return;
            }

            if (moveY != 0)
            {
                MoveRow(moveY);
            }
            if (moveX != 0)
            {
                AdjustValue(moveX);
            }

            if (pad.IsTrigger(enButtonA) || pad.IsTrigger(enButtonB))
            {
                ClosePanel();
            }
        }


        void OptionMenu::MoveItem(const int delta)
        {
            const int index = (static_cast<int>(m_selectedItem) + delta) % ITEM_NUM;
            m_selectedItem = static_cast<EnOptionItem>((index + ITEM_NUM) % ITEM_NUM);

            // 選んでいる行は、選んでいるページの行の数より小さくなければならない。
            // 設定画面を閉じても前の行が残っているので、ページが変わる時に戻す。(行の少ないページで、配列の外を読んでしまう)
            m_selectedRow = 0;
        }


        void OptionMenu::MoveRow(const int delta)
        {
            const int rowNum = PAGES[static_cast<size_t>(m_selectedItem)].m_rowNum;
            const int index = (m_selectedRow + delta) % rowNum;
            m_selectedRow = (index + rowNum) % rowNum;
        }


        void OptionMenu::AdjustValue(const int direction)
        {
            const Page& page = PAGES[static_cast<size_t>(m_selectedItem)];
            StepValue(page.m_rows[m_selectedRow].m_value, direction);
        }


        int OptionMenu::GetSelectedSlot() const
        {
            const Page& page = PAGES[static_cast<size_t>(m_selectedItem)];
            return page.m_rows[m_selectedRow].m_slot;
        }


        void OptionMenu::OpenPanel()
        {
            m_isPanelOpen = true;
            m_selectedRow = 0;
            m_isDirty = true;

            // 前回選んでいた行から滑って見えないように、開く時に合わせておく。
            m_rowTint.Snap(GetSelectedSlot());
        }


        void OptionMenu::ClosePanel()
        {
            m_isPanelOpen = false;
        }


        void OptionMenu::UpdatePanelAlpha(const float deltaTime)
        {
            const float target = m_isPanelOpen ? 1.0f : 0.0f;
            const float step = deltaTime / PANEL_FADE_TIME;
            if (m_panelAlpha < target)
            {
                m_panelAlpha = (std::min)(m_panelAlpha + step, target);
            }
            else
            {
                m_panelAlpha = (std::max)(m_panelAlpha - step, target);
            }
        }


        void OptionMenu::ApplyView()
        {
            const size_t itemIndex = static_cast<size_t>(m_selectedItem);
            const Page& page = PAGES[itemIndex];

            // NOTE: Layoutがjsonの更新でパーツを作り直すと、座標・色・拡大率がjsonの値に戻る。毎フレーム書いて、すぐ直す。
            const UIText* selected = m_items[itemIndex];
            if (m_cursor && selected)
            {
                m_cursor->m_transform.m_localTransform.m_position.y = selected->m_transform.m_localTransform.m_position.y;
            }

            // このページで使う行。値が変わっていたら(デバッグ画面などで外から変わった時も)、文字を書き直す。
            std::array<bool, ROW_SLOT_NUM> isSlotUsed{};
            for (int i = 0; i < page.m_rowNum; ++i)
            {
                const PageRow& row = page.m_rows[i];
                isSlotUsed[row.m_slot] = true;
                if (GetValue(row.m_value) != m_shownValues[row.m_slot])
                {
                    m_isDirty = true;
                }
            }
            if (m_isDirty)
            {
                RefreshTexts();
            }

            // 設定画面の表示。閉じ切ったら描かない。使わない行は描かない。
            const bool isPanelVisible = m_panelAlpha > 0.0f;
            for (const PanelPart& part : m_panelParts)
            {
                part.m_ui->SetIsDraw(isPanelVisible && (part.m_slot < 0 || isSlotUsed[part.m_slot]));
                part.m_ui->m_color.w = part.m_baseAlpha * m_panelAlpha;
            }

            // バーの長さ
            for (int i = 0; i < page.m_rowNum; ++i)
            {
                const PageRow& row = page.m_rows[i];
                if (m_rows[row.m_slot].m_fill)
                {
                    m_rows[row.m_slot].m_fill->m_transform.m_localTransform.m_scale.x = ToRatio(row.m_value, GetValue(row.m_value));
                }
            }

            // 行を選ぶ矢印は、選んでいる行のラベルの高さに付ける。
            if (m_rowCursor)
            {
                const UIText* label = m_rows[GetSelectedSlot()].m_label;
                if (label)
                {
                    m_rowCursor->m_transform.m_localTransform.m_position.y = label->m_transform.m_localTransform.m_position.y;
                }
            }
        }


        void OptionMenu::RefreshTexts()
        {
            const size_t itemIndex = static_cast<size_t>(m_selectedItem);
            const Page& page = PAGES[itemIndex];

            // 見出しは、選んでいる項目の文字
            if (m_panelTitle && m_items[itemIndex])
            {
                m_panelTitle->SetText(m_items[itemIndex]->GetText());
            }

            for (int i = 0; i < page.m_rowNum; ++i)
            {
                const PageRow& row = page.m_rows[i];
                Row& parts = m_rows[row.m_slot];
                const float current = GetValue(row.m_value);

                if (parts.m_label)
                {
                    parts.m_label->SetText(row.m_label);
                }
                if (parts.m_value)
                {
                    wchar_t text[VALUE_TEXT_SIZE];
                    FormatValue(row.m_value, current, text, VALUE_TEXT_SIZE);
                    parts.m_value->SetText(text);
                }
                m_shownValues[row.m_slot] = current;
            }

            m_isDirty = false;
        }
    } // namespace ui
} // namespace app
