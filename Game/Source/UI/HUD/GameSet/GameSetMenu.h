/**
 * @file GameSetMenu.h
 * @brief ゲームセットの文字を動かすMenu
 * @details 画面中央に「GAME SET」を出す。(Assets/parameter/ui/GameSet.json)
 *          jsonに、次の名前のUIText(フォントの文字)を書く。基点は中央(0.5, 0.5)にすると、座標を中心に文字が出る。
 *          - "GameSetText" : 「GAME SET」の文字。
 */
#pragma once
#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief ゲームセットの文字を動かすMenu
         * @details Show() で「GAME SET」を出す。出た瞬間は大きく、少しの間に元の大きさへ縮み、その後は出たままになる。
         *          消す操作は無い。(ゲームセットの後は、シーンごと破棄される)
         */
        class GameSetMenu : public MenuBase
        {
        public:
            GameSetMenu();
            ~GameSetMenu() override;


        public:
            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 文字を取り出して、最初は消しておく。(出している途中で作り直された時は、出したままにする)
             */
            void InitializeLogic() override;

            /**
             * @brief 更新
             * @details 出している間は、時間を進めて、大きさの動きを文字に反映する。縮み終わった後は、時間を進めない。
             */
            void Update() override;


        public:
            /**
             * @brief 「GAME SET」を出す
             * @details すでに出している時は、何もしない。(動きをやり直さない)
             */
            void Show();


        private:
            /** 出た瞬間の大きさの動きを、文字に反映する */
            void ApplyPop();


        private:
            /** 「GAME SET」の文字。(所有はキャンバス) */
            UIText* m_text;
            /** 出している間かどうか */
            bool m_isShowing;
            /** 出してからの経過時間(秒)。出している間だけ進み、縮み終わったら止まる */
            float m_elapsed;
        };
    } // namespace ui
} // namespace app
