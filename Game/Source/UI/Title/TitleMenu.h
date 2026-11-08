/**
 * @file TitleMenu.h
 * @brief タイトル画面のUIを動かすMenu
 * @details jsonに、次の名前のUIText(フォントの文字)を書く。(Assets/parameter/ui/Title.json)
 *          それ以外のパーツ(背景・ロゴ)は動かさないので、コードでは取り出さない。
 *          - "ItemStart" / "ItemOption" / "ItemQuit" : 項目の文字。EnTitleItem の順に、上から並べる。
 *            jsonの色が、選んでいる時の色。選んでいない項目は、薄灰色に変わる(SelectionTint)
 *          - "Cursor" : 選択カーソルの矢印。x座標はjsonの値を中心に左右に揺らし(AttachCursorSway)、y座標を選んでいる項目に合わせる
 */
#pragma once
#include <array>
#include <cstdint>

#include "Source/UI/Menu.h"
#include "Source/UI/SelectionTint.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief タイトル画面の項目。上から並ぶ順
         */
        enum class EnTitleItem : uint8_t
        {
            Start,  //!< はじめる
            Option, //!< オプション
            Quit,   //!< おわる
            Max
        };


        /**
         * @brief タイトル画面のUIを動かすMenu
         * @details 項目の中から1つを選ぶ。選択カーソル(矢印)は、選んでいる項目の高さに付いてくる。
         *          入力は受け付けない。選ぶ項目は、MoveSelection() で動かす。
         */
        class TitleMenu : public MenuBase
        {
        public:
            TitleMenu();
            ~TitleMenu() override;


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
             * @brief 選んでいる項目を動かす
             * @details 端を越えたら、反対の端に回る。
             * @param delta 動かす数。+1で1つ下、-1で1つ上
             */
            void MoveSelection(const int delta);

            /**
             * @brief 選んでいる項目を取得
             * @return 選んでいる項目
             */
            EnTitleItem GetSelectedItem() const
            {
                return m_selectedItem;
            }


        private:
            /** 項目の文字。EnTitleItem の順。(所有はキャンバス) */
            std::array<UIText*, static_cast<size_t>(EnTitleItem::Max)> m_items;
            /** 選択カーソルの矢印。(所有はキャンバス) */
            UIText* m_cursor;
            /** 選んでいない項目を薄灰色にする。選択肢の番号は EnTitleItem の値 */
            SelectionTint m_tint;
            /** 選んでいる項目 */
            EnTitleItem m_selectedItem;
        };
    } // namespace ui
} // namespace app
