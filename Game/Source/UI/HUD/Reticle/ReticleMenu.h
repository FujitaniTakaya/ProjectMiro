/**
 * @file ReticleMenu.h
 * @brief レティクル(照準の十字)を動かすMenu
 * @details 画面中央に、上下左右の4本のバーと中心の点を出す。(Assets/parameter/ui/Reticle.json)
 *          jsonに、次の名前のUIIcon(バー)を書く。バーの位置が、構えていない時の広がり具合になる。
 *          - "ReticleTop" / "ReticleBottom" / "ReticleLeft" / "ReticleRight" : 4本のバー。
 *          - "ReticleDot" : 中心の点。(動かさず、色だけ変える)
 *          構えると、バーが中央に集まる。弾を撃つと、バーが外に広がって、すぐ元に戻る。
 *          ゾンビを指している間は、バーと点が赤くなる。
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
         *          SetOnTarget() でゾンビを指しているかを受け取って、色を変える。
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

            /**
             * @brief ゾンビを指しているかを設定
             * @details 指している間、4本のバーと中心の点が赤くなる。(アルファはjsonのまま)
             * @param isOnTarget ゾンビを指しているなら true
             */
            void SetOnTarget(const bool isOnTarget);


        private:
            /** 今の広がり具合を、4本のバーの位置に反映する */
            void ApplySpread();

            /** 今の状態(ゾンビを指しているか)を、パーツの色に反映する */
            void ApplyColor();


        private:
            /** バーの数(上・下・左・右) */
            static constexpr int BAR_COUNT = 4;
            /** 色を変えるパーツの数(4本のバー + 中心の点) */
            static constexpr int PART_COUNT = BAR_COUNT + 1;

            /** 色を変えるパーツ。先頭の4つがバー(上・下・左・右)、最後が中心の点。(所有はキャンバス) */
            UIIcon* m_parts[PART_COUNT];
            /** パーツの、jsonの色(ゾンビを指していない時の色) */
            Vector4 m_baseColors[PART_COUNT];
            /** バー4本の、jsonの位置(構えていない時の位置) */
            Vector3 m_basePositions[BAR_COUNT];
            /** 構えの度合い(0=通常、1=構え切った状態) */
            float m_aimRate;
            /** 構えているか */
            bool m_isAiming;
            /** 発砲で足した広がり(0に向かって減っていく) */
            float m_kick;
            /** ゾンビを指しているか */
            bool m_isOnTarget;
        };
    } // namespace ui
} // namespace app
