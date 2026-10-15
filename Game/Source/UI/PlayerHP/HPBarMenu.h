/**
 * @file HPBarMenu.h
 * @brief HPバーを動かすMenu
 * @details jsonに、次の名前のUIGaugeを、灰色(背景)→赤→緑の順に並べて書く。(Assets/parameter/ui/HPBar.json)
 *          UIGaugeの"pivot"を左端([0, 0.5])にしておくと、拡大率のxを変えるだけで、右から減るバーになる。
 *          - "GreyGauge"  : 背景。動かさないので、コードでは取り出さない
 *          - "RedGauge"   : ダメージの残像。緑より遅れて減る
 *          - "GreenGauge" : 現在のHP
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief HPバーを動かすMenu
         * @details SetHp() で現在のHPを受け取る。緑のバーは、すぐに現在のHPの割合になる。
         *          赤のバーは、緑のバーより長い間だけ、時間をかけて緑のバーに追いつく。回復した時は、すぐ緑のバーに揃う。
         */
        class HPBarMenu : public MenuBase
        {
        public:
            HPBarMenu();
            ~HPBarMenu() override;


        public:
            /** 更新 */
            void Update() override;

            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 緑と赤のバーを取り出して、今の割合を反映する。
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief HPを設定
             * @param hp 現在のHP
             * @param maxHp 最大HP。0以下の場合は、バーは空になる
             */
            void SetHp(const float hp, const float maxHp);

            /**
             * @brief 緑のバーの割合を取得
             * @return 現在のHPの割合(0〜1)
             */
            float GetGreenRatio() const
            {
                return m_greenRatio;
            }

            /**
             * @brief 赤のバーの割合を取得
             * @return 赤のバーの割合(0〜1)。緑のバー以上
             */
            float GetRedRatio() const
            {
                return m_redRatio;
            }


        private:
            /** 緑・赤のバーの拡大率を、今の割合にする */
            void ApplyRatio();


        private:
            /** 赤のバー。(所有はキャンバス) */
            UIGauge* m_redGauge;
            /** 緑のバー。(所有はキャンバス) */
            UIGauge* m_greenGauge;
            /** 緑のバーの割合(0〜1) */
            float m_greenRatio;
            /** 赤のバーの割合(0〜1) */
            float m_redRatio;
        };
    } // namespace ui
} // namespace app
