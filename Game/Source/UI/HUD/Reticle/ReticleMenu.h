/**
 * @file ReticleMenu.h
 * @brief レティクル(照準の十字)を動かすMenu
 * @details 画面中央に、上下左右の4本のバーと中心の点を出す。(Assets/parameter/ui/Reticle.json)
 *          jsonに、次の名前のUIIcon(バー)を書く。バーの位置が、構えていない時の広がり具合になる。
 *          - "ReticleTop" / "ReticleBottom" / "ReticleLeft" / "ReticleRight" : 4本のバー。
 *          構えると、バーが中央に集まる。弾を撃つと、バーが外に広がって、すぐ元に戻る。
 */
#pragma once
#include "Source/UI/Foundation/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief レティクルを動かすMenu
         * @details SetAiming() で構えの切り替え、OnFired() で発砲を受け取って、4本のバーの位置を毎フレーム動かす。
         */
        class ReticleMenu : public MenuBase
        {
        public:
            ReticleMenu();
            ~ReticleMenu() override;


        public:
            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 4本のバーを取り出し、jsonの位置を基準として覚えて、今の広がり具合を反映する。
             */
            void InitializeLogic() override;

            /**
             * @brief 更新
             * @details 構えの度合いと発砲の広がりを動かして、バーの位置に反映する。
             */
            void Update() override;


        public:
            /**
             * @brief 構えているかどうかを設定
             * @details 構えている間は、バーが中央に集まる。
             * @param isAiming 構えているならtrue
             */
            void SetAiming(const bool isAiming);

            /**
             * @brief 弾を撃ったことを知らせる
             * @details バーが外に広がり、少しずつ元に戻る。連射すると広がりが足されるが、上限がある。
             */
            void OnFired();


        private:
            /** 今の広がり具合を、4本のバーの位置に反映する */
            void ApplySpread();


        private:
            /** バーの数(上・下・左・右) */
            static constexpr int BAR_COUNT = 4;

            /** 4本のバー。(所有はキャンバス) */
            UIIcon* m_bars[BAR_COUNT];
            /** 4本のバーの、jsonの位置(構えていない時の位置) */
            Vector3 m_basePositions[BAR_COUNT];
            /** 構えの度合い(0=通常、1=構え切った状態) */
            float m_aimRate;
            /** 構えているか */
            bool m_isAiming;
            /** 発砲で足した広がり(0に向かって減っていく) */
            float m_kick;
        };
    } // namespace ui
} // namespace app
