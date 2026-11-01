/**
 * @file TitleMenu.h
 * @brief タイトル画面のUIを動かすMenu
 * @details jsonに、次の名前のUIText(フォントの文字)を書く。(Assets/parameter/ui/Title.json)
 *          それ以外のパーツ(背景・ロゴ・サブタイトル)は動かさないので、コードでは取り出さない。
 *          - "PressStart" : 「PRESS START」。ゆっくり点滅させる
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief タイトル画面のUIを動かすMenu
         * @details 「PRESS START」の文字を、透明度を変えて点滅させる。
         */
        class TitleMenu : public MenuBase
        {
        public:
            TitleMenu();
            ~TitleMenu() override;


        public:
            /** 更新。「PRESS START」の点滅を進める。 */
            void Update() override;

            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 「PRESS START」の文字を取り出す。
             */
            void InitializeLogic() override;


        private:
            /** 「PRESS START」の文字。(所有はキャンバス) */
            UIText* m_pressStart;
            /** 点滅の経過時間(秒)。点滅の周期の中で繰り返す */
            float m_blinkTimer;
        };
    } // namespace ui
} // namespace app
