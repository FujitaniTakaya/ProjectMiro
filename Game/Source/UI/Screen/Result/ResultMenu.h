/**
 * @file ResultMenu.h
 * @brief リザルト画面のUIを動かすMenu
 * @details jsonに、次の名前のUIText(フォントの文字)を書く。(Assets/parameter/ui/Result.json)
 *          - "ItemTitle" / "ItemQuit" : 項目の文字。EnResultItem の順に、上から並べる。
 *            jsonの色が、選んでいる時の色。選んでいない項目は、薄灰色に変わる(SelectionTint)
 *          - "Cursor" : 選択カーソルの矢印。x座標はjsonの値を中心に左右に揺らし(AttachCursorSway)、y座標を選んでいる項目に合わせる
 *          スコアの表示は、次の名前のUIText。数字の文字は SetResult() で書き込む。基点を右(x=1)にすると、右端が座標に揃う。
 *          - "TimeValue" / "NormalValue" / "RunningValue" : 守衛時間(m:ss)・倒した通常ゾンビの数・倒したランニングゾンビの数(EnResultRow の順)
 *          - "TimePoints" / "NormalPoints" / "RunningPoints" : それぞれの点
 *          - "TotalPoints" : 合計(スコア)
 *          行の名前の文字(「守衛時間」など)は、jsonに固定の文字で書く。
 */
#pragma once
#include <array>
#include <cstdint>

#include "Source/Score/GameResult.h"
#include "Source/Score/ScoreRule.h"
#include "Source/UI/Common/SelectionTint.h"
#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief リザルト画面の項目。上から並ぶ順
         */
        enum class EnResultItem : uint8_t
        {
            Title, //!< タイトルへ
            Quit,  //!< ゲームをやめる
            Max
        };


        /**
         * @brief スコアの内訳の行。上から並ぶ順
         */
        enum class EnResultRow : uint8_t
        {
            Time,    //!< 守衛時間
            Normal,  //!< 倒した通常ゾンビ
            Running, //!< 倒したランニングゾンビ
            Max
        };


        /**
         * @brief リザルト画面のUIを動かすMenu
         * @details 項目の中から1つを選ぶ。選択カーソル(矢印)は、選んでいる項目の高さに付いてくる。
         *          入力は受け付けない。選ぶ項目は、MoveSelection() で動かす。
         */
        class ResultMenu : public MenuBase
        {
        public:
            ResultMenu();
            ~ResultMenu() override;


        public:
            /** 更新。選択カーソルを、選んでいる項目の高さに合わせて、選んでいない項目を薄灰色にする。 */
            void Update() override;

            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 項目の文字と、選択カーソルを取り出して、今の選択をすぐ反映する。
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief 結果(守衛時間・倒した数・スコア)を設定
             * @details 数字の文字を書き込む。UIのパーツが作り直されても(InitializeLogic())、同じ結果を書き直す。
             *          設定するまでは、jsonに書かれた文字のまま。
             * @param data インゲームの結果
             * @param score data から計算したスコアの内訳
             */
            void SetResult(const GameResultData& data, const ScoreBreakdown& score);

            /**
             * @brief 選んでいる項目を動かす
             * @details 端を越えたら、反対の端に回る。
             * @param delta 動かす数。+1で1つ下、-1で1つ上
             */
            void MoveSelection(const int delta);

            /**
             * @brief 選んでいる項目を取得
             * @return 選んでいる項目
             */
            EnResultItem GetSelectedItem() const
            {
                return m_selectedItem;
            }


        private:
            /** 文字に、今の結果を反映する(結果を設定済みの時だけ) */
            void ApplyResult();


        private:
            /** 項目の文字。EnResultItem の順。(所有はキャンバス) */
            std::array<UIText*, static_cast<size_t>(EnResultItem::Max)> m_items;
            /** 選択カーソルの矢印。(所有はキャンバス) */
            UIText* m_cursor;
            /** 選んでいない項目を薄灰色にする。選択肢の番号は EnResultItem の値 */
            SelectionTint m_tint;
            /** 選んでいる項目。最初は「タイトルへ」 */
            EnResultItem m_selectedItem;
            /** 内訳の行の数字の文字(守衛時間・倒した数)。EnResultRow の順。(所有はキャンバス) */
            std::array<UIText*, static_cast<size_t>(EnResultRow::Max)> m_valueTexts;
            /** 内訳の行の点の文字。EnResultRow の順。(所有はキャンバス) */
            std::array<UIText*, static_cast<size_t>(EnResultRow::Max)> m_pointsTexts;
            /** 合計(スコア)の文字。(所有はキャンバス) */
            UIText* m_totalText;
            /** インゲームの結果 */
            GameResultData m_data;
            /** スコアの内訳 */
            ScoreBreakdown m_score;
            /** 結果を設定済みか */
            bool m_hasResult;
        };
    } // namespace ui
} // namespace app
