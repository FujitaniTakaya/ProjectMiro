/**
 * @file GameTimerMenu.h
 * @brief インゲームの残り時間を動かすMenu
 * @details 画面右上に、残り時間を「分:秒」で出す。(Assets/parameter/ui/GameTimer.json)
 *          jsonに、次の名前のUIText(フォントの文字)を書く。基点を右(x=1)にすると、右端が座標に揃う。
 *          - "TimerText" : 残り時間の文字。
 */
#pragma once
#include <cstdint>

#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief インゲームの残り時間を動かすMenu
         * @details SetRemainingTime() で残り時間(秒)を受け取って、「m:ss」の文字列に反映する。
         *          秒は切り上げて表示する。(残り 0.1 秒の間は 0:01 のまま、ちょうど 0 になった時に 0:00 になる)
         */
        class GameTimerMenu : public MenuBase
        {
        public:
            GameTimerMenu();
            ~GameTimerMenu() override;


        public:
            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 文字を取り出して、今の残り時間を反映する。
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief 残り時間を設定
             * @details 表示する秒(切り上げ)が変わらない間は、文字を作り直さない。
             * @param seconds 残り時間(秒)。負の値は0として扱う
             */
            void SetRemainingTime(const float seconds);


        private:
            /** 文字に、今の残り時間を反映する */
            void ApplyTime();


        private:
            /** 残り時間の文字。(所有はキャンバス) */
            UIText* m_timerText;
            /** 表示している秒(切り上げた整数)。まだ一度も設定されていない間は -1 */
            int32_t m_displaySeconds;
        };
    } // namespace ui
} // namespace app
