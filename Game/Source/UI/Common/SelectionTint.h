/**
 * @file SelectionTint.h
 * @brief 選んでいない選択肢の色を、薄灰色にする
 * @details jsonに書いた色を「選んでいる時の色」とする。選んでいない選択肢は、RGBを暗くして薄灰色にする。透明度は触らない。
 *          1つの選択肢に、複数のUIのパーツを登録できる。(設定画面の1行は、ラベル・バー・値の3つ)
 *          選び方が別々に動く画面は、SelectionTintを別々に持つこと。
 *          例: m_tint.Clear();                  // MenuBase::InitializeLogic() の中で、パーツを取り出した後に登録し直す
 *              m_tint.AddPart(0, m_items[0]);
 *              m_tint.Snap(selected);
 *              m_tint.Step(selected, deltaTime);  // MenuBase::Update() より前に、毎フレーム呼ぶ
 */
#pragma once
#include <vector>

#include "Source/UI/Foundation/UIParts.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief 選んでいない選択肢の色を、薄灰色にする
         * @details 選択肢ごとに明るさ(1で元の色、0で薄灰色)を持ち、選んでいる選択肢に向けて少しずつ動かす。
         *          色は UIBase::m_color のRGBに書く。
         */
        class SelectionTint
        {
        public:
            SelectionTint();
            ~SelectionTint();


        public:
            /**
             * @brief 登録されているパーツを全て外す
             * @details Layoutがjsonの更新でUIのパーツを作り直すと、登録したポインタが無効になる。
             *          MenuBase::InitializeLogic() の度に、最初に呼ぶこと。
             */
            void Clear();

            /**
             * @brief 選択肢にパーツを登録する
             * @details 登録した時の色を、選んでいる時の色とする。作り直された直後(jsonの色のまま)に呼ぶこと。
             * @param choice 選択肢の番号(0から)
             * @param ui 登録するパーツ。nullptrの場合は何もしない。
             */
            void AddPart(const int choice, UIBase* ui);

            /**
             * @brief 選んでいる選択肢に向けて明るさを動かして、色に書く
             * @details 選んでいる選択肢は元の色に、それ以外は薄灰色に向かう。
             * @param selected 選んでいる選択肢の番号。どれにも当たらない値なら、全て薄灰色に向かう。
             * @param deltaTime 前のフレームからの経過時間(秒)
             */
            void Step(const int selected, const float deltaTime);

            /**
             * @brief 明るさを動かさずに、選んでいる選択肢に合わせて、色に書く
             * @details 前回の途中の色を見せたくない時に使う。(画面を開いた時、UIが作り直された時)
             * @param selected 選んでいる選択肢の番号。どれにも当たらない値なら、全て薄灰色にする。
             */
            void Snap(const int selected);


        private:
            /**
             * @brief 登録されたパーツ
             */
            struct Part
            {
                /** パーツ。(所有はキャンバス) */
                UIBase* m_ui;
                /** 選んでいる時の色。登録した時の色 */
                Vector4 m_baseColor;
                /** 属する選択肢の番号 */
                int m_choice;
            };


        private:
            /** 選択肢の明るさを、パーツの色に書く。透明度は書かない */
            void Apply();


        private:
            /** 登録されたパーツ全て */
            std::vector<Part> m_parts;
            /** 選択肢ごとの明るさ。1で元の色、0で薄灰色。選択肢の番号の順 */
            std::vector<float> m_amounts;
        };
    } // namespace ui
} // namespace app
