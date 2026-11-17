/**
 * @file CountdownMenu.h
 * @brief ゲーム開始前のカウントダウンの数字を動かすMenu
 * @details 画面中央に、カウントダウンの数字(3,2,1)と、その後の「Start」を出す。(Assets/parameter/ui/Countdown.json)
 *          jsonに、次の名前のUIText(フォントの文字)を書く。基点は中央(0.5, 0.5)にすると、座標を中心に文字が出る。
 *          - "CountdownNumber" : カウントダウンの数字と「Start」。同じUITextの文字を入れ替えて使う。
 */
#pragma once
#include <cstdint>

#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief ゲーム開始前のカウントダウンの数字を動かすMenu
         * @details Show() で数字を受け取って表示する。数字が出た瞬間は大きく、少しの間に元の大きさへ縮む。
         *          ShowStart() で「Start」を出す。「Start」は、出してから一定の時間が経つと、Update() の中で自分で消える。
         *          Hide() で、すぐに消す。数字・「Start」を出すタイミングは、持ち主(シーン)が決める。
         */
        class CountdownMenu : public MenuBase
        {
        public:
            CountdownMenu();
            ~CountdownMenu() override;


        public:
            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 数字を取り出して、最初は消しておく。
             */
            void InitializeLogic() override;

            /**
             * @brief 更新
             * @details 「Start」を出している間は、時間を進めて、大きさの動きと、消えるタイミングを決める。
             */
            void Update() override;


        public:
            /**
             * @brief 数字を出す
             * @details 「Start」を出している途中なら、数字に切り替わる。
             * @param number 出す数字
             * @param elapsed その数字を出してからの経過時間(秒)。出た瞬間の動きに使う
             */
            void Show(const int32_t number, const float elapsed);

            /**
             * @brief 「Start」を出す
             * @details 出した瞬間は大きく、少しの間に元の大きさへ縮む。一定の時間が経つと、Update() の中で消える。
             */
            void ShowStart();

            /** 数字・「Start」を消す */
            void Hide();


        private:
            /**
             * @brief 出た瞬間の大きさの動きを、文字に反映する
             * @param baseScale 元の大きさの倍率
             * @param elapsed 文字を出してからの経過時間(秒)
             */
            void ApplyPop(const float baseScale, const float elapsed);


        private:
            /** カウントダウンの数字と「Start」。(所有はキャンバス) */
            UIText* m_number;
            /** 「Start」を出している間かどうか */
            bool m_isShowingStart;
            /** 「Start」を出してからの経過時間(秒)。出している間だけ進む */
            float m_startElapsed;
        };
    } // namespace ui
} // namespace app
