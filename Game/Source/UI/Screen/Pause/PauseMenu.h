/**
 * @file PauseMenu.h
 * @brief ポーズ画面のUIを動かすMenu
 * @details jsonに、次の名前のUIを書く。(Assets/parameter/ui/Pause.json)
 *          - "Backdrop" : 画面を少し暗くする板。透明度だけを、画面のフェードに合わせて動かす
 *          - "Heading" : 見出し。透明度だけを、画面のフェードに合わせて動かす
 *          - "ItemOption" / "ItemTitle" : 項目の文字。EnPauseItem の順に、上から並べる。
 *            jsonの色が、選んでいる時の色。選んでいない項目は、薄灰色に変わる(SelectionTint)
 *          - "Cursor" : 選択カーソルの矢印。x座標はjsonの値を中心に左右に揺らし(AttachCursorSway)、y座標を選んでいる項目に合わせる
 */
#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "Source/UI/Common/SelectionTint.h"
#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief ポーズ画面の項目。上から並ぶ順
         */
        enum class EnPauseItem : uint8_t
        {
            Option, //!< オプション
            Title,  //!< タイトルへ
            Max
        };


        /**
         * @brief ポーズ画面のUIを動かすMenu
         * @details 項目の中から1つを選ぶ。選択カーソル(矢印)は、選んでいる項目の高さに付いてくる。
         *          入力は受け付けない。選ぶ項目は、MoveSelection() で動かす。
         *          画面全体の透明度は、SetAlpha() で動かす。ポーズに入る・出るフェードに使う。
         */
        class PauseMenu : public MenuBase
        {
        public:
            PauseMenu();
            ~PauseMenu() override;


        public:
            /** 更新。選択カーソル・選んでいない項目の薄灰色・全体の透明度を、UIに反映する。 */
            void Update() override;

            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 項目の文字・選択カーソル・透明度を動かすパーツを取り出して、今の状態をすぐ反映する。
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief 画面を開く
             * @details 選んでいる項目を先頭に戻して、すぐUIに反映する。ポーズに入る度に呼ぶ。
             *          (呼んだフレームのうちに描かれても、前回の選択や、薄灰色に変わる途中の色が映らない)
             */
            void Open();

            /**
             * @brief 選んでいる項目を動かす
             * @details 端を越えたら、反対の端に回る。
             * @param delta 動かす数。+1で1つ下、-1で1つ上
             */
            void MoveSelection(const int delta);

            /**
             * @brief 画面全体の透明度を設定する
             * @details 各パーツの透明度は、jsonの値にこの値を掛けたものになる。すぐUIに反映する。
             * @param alpha 透明度(0〜1)。0で見えなくなる
             */
            void SetAlpha(const float alpha);

            /**
             * @brief 選んでいる項目を取得
             * @return 選んでいる項目
             */
            EnPauseItem GetSelectedItem() const
            {
                return m_selectedItem;
            }


        private:
            /**
             * @brief 透明度を動かすパーツ
             */
            struct FadePart
            {
                /** パーツ。(所有はキャンバス) */
                UIBase* m_ui;
                /** jsonの透明度(0〜1) */
                float m_baseAlpha;
            };


        private:
            /**
             * @brief 透明度を動かすパーツに登録する
             * @param ui 登録するパーツ。nullptrなら何もしない
             */
            void AddFadePart(UIBase* ui);

            /**
             * @brief 状態を、UIのパーツに反映する
             * @details カーソルの位置と透明度。Layoutがjsonの更新でパーツを作り直すと、値がjsonに戻るので、毎フレーム書く。
             */
            void ApplyView();


        private:
            /** 項目の文字。EnPauseItem の順。(所有はキャンバス) */
            std::array<UIText*, static_cast<size_t>(EnPauseItem::Max)> m_items;
            /** 選択カーソルの矢印。(所有はキャンバス) */
            UIText* m_cursor;
            /** 選んでいない項目を薄灰色にする。選択肢の番号は EnPauseItem の値 */
            SelectionTint m_tint;
            /** 透明度を動かすパーツ全て */
            std::vector<FadePart> m_fadeParts;
            /** 選んでいる項目 */
            EnPauseItem m_selectedItem;
            /** 画面全体の透明度(0〜1) */
            float m_alpha;
        };
    } // namespace ui
} // namespace app
