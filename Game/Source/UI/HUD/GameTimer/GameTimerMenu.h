/**
 * @file GameTimerMenu.h
 * @brief インゲームの経過時間を動かすMenu
 * @details 画面右上に、経過時間を「分:秒」で出す。(Assets/parameter/ui/GameTimer.json)
 *          jsonに、次の名前のUIText(フォントの文字)を書く。基点を右(x=1)にすると、右端が座標に揃う。
 *          - "TimerText" : 経過時間の文字。
 */
#pragma once
#include <cstdint>

#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief インゲームの経過時間を動かすMenu
         * @details SetElapsedTime() で経過時間(秒)を受け取って、「m:ss」の文字列に反映する。
         *          秒は切り捨てて表示する。(始まってから 1 秒になるまでは 0:00 のまま)
         */
        class GameTimerMenu : public MenuBase
        {
        public:
            GameTimerMenu();
            ~GameTimerMenu() override;


        public:
            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 文字を取り出して、今の経過時間を反映する。
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief 経過時間を設定
             * @details 表示する秒(切り捨て)が変わらない間は、文字を作り直さない。
             * @param seconds 経過時間(秒)。負の値は0として扱う
             */
            void SetElapsedTime(const float seconds);


        private:
            /** 文字に、今の経過時間を反映する */
            void ApplyTime();


        private:
            /** 経過時間の文字。(所有はキャンバス) */
            UIText* m_timerText;
            /** 表示している秒(切り捨てた整数)。まだ一度も設定されていない間は -1 */
            int32_t m_displaySeconds;
        };
    } // namespace ui
} // namespace app
