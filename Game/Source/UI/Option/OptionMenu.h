/**
 * @file OptionMenu.h
 * @brief オプション画面のUIを動かすMenu
 * @details jsonに、次の名前のUIを書く。(Assets/parameter/ui/Option.json)
 *          それ以外のパーツ(背景・見出し)は動かさないので、コードでは取り出さない。
 *          - "ItemSound" / "ItemSensitivity" : 項目の文字。EnOptionItem の順に、上から並べる。
 *            jsonの色が、選んでいる時の色。選んでいない項目は、薄灰色に変わる(SelectionTint)
 *          - "Cursor" : 項目を選ぶ矢印。x座標はjsonの値を中心に左右に揺らし(AttachCursorSway)、y座標を選んでいる項目に合わせる
 *          - "Panel" : 設定画面の板。画面の中央から左側に置く
 *          - "PanelTitle" : 設定画面の見出し。選んでいる項目の文字が入る
 *          - "Row0Label" "Row0BarBg" "Row0BarFill" "Row0Value" : 設定画面の1行目(2行目は Row1、3行目は Row2)。
 *            BarFill は UIGauge で、基点を左端にする。拡大率xが値の割合になる。
 *            Label・BarFill・Value は、jsonの色が、選んでいる行の色。選んでいない行は、薄灰色に変わる(BarBg は変わらない)
 *          - "RowCursor" : 設定画面で、行を選ぶ矢印。x座標はjsonの値を中心に左右に揺らし(AttachCursorSway)、y座標を選んでいる行のラベルに合わせる
 *          サウンドは3行(Row0〜Row2)、感度は2行(Row0・Row1)を使う。
 */
#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "Source/UI/Menu.h"
#include "Source/UI/MenuInput.h"
#include "Source/UI/SelectionTint.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief オプション画面の項目。上から並ぶ順
         */
        enum class EnOptionItem : uint8_t
        {
            Sound,       //!< サウンド(全体・BGM・SEの音量の3行)
            Sensitivity, //!< 感度(カメラ感度と、ハンドガンを構えている時のエイム感度の2行)
            Max
        };


        /**
         * @brief オプション画面のUIを動かすMenu
         * @details 項目の一覧と、選んだ項目の設定画面を持つ。入力はこのクラスが読む。
         *          - 一覧 : 上下で項目を選ぶ。A(Start)で設定画面を開く。Bで閉じる(IsClosed()がtrueになる)
         *          - 設定画面 : 上下で行を選び、左右で値を動かす。AかBで一覧に戻る
         *          値は動かした瞬間に反映する。音量は SoundManager、感度は OptionSettings に書く。
         */
        class OptionMenu : public MenuBase
        {
        public:
            OptionMenu();
            ~OptionMenu() override;


        public:
            /** 更新。入力を読んで、設定画面の開閉と値を動かし、UIに反映する。 */
            void Update() override;

            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 項目の文字・設定画面のパーツを取り出して、今の状態(薄灰色の色も)をすぐ反映する。(作り直されたパーツは、jsonの値のまま全て描かれる状態になっている)
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief 画面を開く
             * @details 選んでいる項目を先頭に戻し、設定画面を閉じ、IsClosed() を false に戻す。オプション画面に入る度に呼ぶ。
             *          見た目にも、すぐ反映する。(呼んだフレームのうちに描かれても、前回の見た目や、薄灰色に変わる途中の色が映らない。Update() の入力は、次のフレームから受け取る)
             */
            void Open();

            /**
             * @brief 画面を閉じたかどうか
             * @details 一覧でBを押すと true になる。Open() で false に戻る。
             * @return 閉じたらtrue
             */
            bool IsClosed() const
            {
                return m_isClosed;
            }


        public:
            /** 設定画面の行の数(1ページに出せる行の最大数) */
            static constexpr int ROW_SLOT_NUM = 3;


        private:
            /**
             * @brief 設定画面の1行のパーツ
             */
            struct Row
            {
                /** ラベル。(所有はキャンバス) */
                UIText* m_label;
                /** 値のバー。(所有はキャンバス) */
                UIGauge* m_fill;
                /** 値の文字。(所有はキャンバス) */
                UIText* m_value;
            };

            /**
             * @brief 設定画面に含まれるパーツ。開閉のフェードで、透明度を変える
             */
            struct PanelPart
            {
                /** パーツ。(所有はキャンバス) */
                UIBase* m_ui;
                /** jsonの透明度(0〜1) */
                float m_baseAlpha;
                /** 属する行の番号。どの行にも属さない(常に出す)パーツは-1 */
                int m_slot;
            };


        private:
            /**
             * @brief 設定画面のパーツを取り出して、PanelPart に登録する
             * @tparam T 取り出すパーツの型
             * @param key パーツの名前(ハッシュ値)
             * @param slot 属する行の番号。どの行にも属さないなら-1
             * @return パーツ。無い場合はnullptr
             */
            template <typename T>
            T* FindPanelPart(const uint32_t key, const int slot);

            /**
             * @brief 入力を読んで、画面を動かす
             * @param pad 入力を受け取るパッド
             */
            void HandleInput(const GamePad& pad);

            /**
             * @brief 一覧で、選んでいる項目を動かす
             * @details 端を越えたら、反対の端に回る。設定画面で選んでいる行は、先頭に戻す。
             * @param delta 動かす数。+1で1つ下、-1で1つ上
             */
            void MoveItem(const int delta);

            /**
             * @brief 設定画面で、選んでいる行を動かす
             * @details 端を越えたら、反対の端に回る。
             * @param delta 動かす数。+1で1つ下、-1で1つ上
             */
            void MoveRow(const int delta);

            /**
             * @brief 選んでいる行の値を、1段階動かす
             * @param direction -1で下げる、+1で上げる
             */
            void AdjustValue(const int direction);

            /**
             * @brief 設定画面で選んでいる行の、行の番号(jsonの Row0〜Row2)を取得する
             * @return 行の番号(0〜ROW_SLOT_NUM - 1)
             */
            int GetSelectedSlot() const;

            /** 選んでいる項目の設定画面を開く */
            void OpenPanel();

            /** 設定画面を閉じて、一覧に戻る */
            void ClosePanel();

            /**
             * @brief 設定画面の透明度を、開閉に合わせて動かす
             * @param deltaTime 前のフレームからの経過時間(秒)
             */
            void UpdatePanelAlpha(const float deltaTime);

            /**
             * @brief 状態を、UIのパーツに反映する
             * @details カーソルの位置・設定画面の表示・バー・文字。Layoutがjsonの更新でパーツを作り直すと、値がjsonに戻るので、毎フレーム書く。
             */
            void ApplyView();

            /** 見出し・ラベル・値の文字を、選んでいるページに合わせて書き直す */
            void RefreshTexts();


        private:
            /** 項目の文字。EnOptionItem の順。(所有はキャンバス) */
            std::array<UIText*, static_cast<size_t>(EnOptionItem::Max)> m_items;
            /** 項目を選ぶ矢印。(所有はキャンバス) */
            UIText* m_cursor;
            /** 設定画面の見出し。(所有はキャンバス) */
            UIText* m_panelTitle;
            /** 設定画面の、行を選ぶ矢印。(所有はキャンバス) */
            UIText* m_rowCursor;
            /** 設定画面の行 */
            std::array<Row, ROW_SLOT_NUM> m_rows;
            /** 選んでいない項目を薄灰色にする。選択肢の番号は EnOptionItem の値 */
            SelectionTint m_itemTint;
            /** 選んでいない行を薄灰色にする。選択肢の番号は行の番号(ラベル・バー・値をまとめて変える) */
            SelectionTint m_rowTint;
            /** 設定画面に含まれるパーツ全て */
            std::vector<PanelPart> m_panelParts;
            /** 文字に書いた時の値(行ごと)。値が変わったことに気付いて、文字を書き直すために持つ */
            std::array<float, ROW_SLOT_NUM> m_shownValues;
            /** 方向入力 */
            MenuInput m_input;
            /** 選んでいる項目 */
            EnOptionItem m_selectedItem;
            /** 設定画面で選んでいる行(ページの行の中の番号)。設定画面を閉じている間も、選んでいるページの行の数より小さく保つ */
            int m_selectedRow;
            /** 設定画面の透明度(0〜1)。開くと1に向かい、閉じると0に向かう */
            float m_panelAlpha;
            /** 設定画面が開いているかどうか */
            bool m_isPanelOpen;
            /** 画面を閉じたかどうか */
            bool m_isClosed;
            /** 文字を書き直す必要があるかどうか */
            bool m_isDirty;
        };
    } // namespace ui
} // namespace app
