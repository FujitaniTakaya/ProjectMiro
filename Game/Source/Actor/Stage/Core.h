/**
 * @file Core.h
 * @brief ステージのコア
 */
#pragma once
#include "Source/Actor/Actor.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ステージのコア
         * @details 宝石のモデル(Gemme.tkm)を描画する。
         *          地面から少し浮いた位置で、X軸まわりに少し傾いたまま、自身の軸で常に回転する。
         *          SetPosition() には、コアの真下の地面の座標を渡す(浮かせる高さは Core 側で足す)。
         *          SetRotation() で設定した回転は、毎フレームの回転で上書きされる。
         *          Start() を一度呼んでから、毎フレーム Update() と Render() を呼ぶ。
         * @note モデルは Assets/modelData/stage/core/Gemme.tkm(Z-up・アニメーション無し)。
         *       モデルはメートル単位で書き出されている(高さ1.32)。ワールドはcm単位なので、拡大率の初期値は100倍にしてあり、さらに1.6倍している。
         *       モデルの原点は中心から少しずれているので、宝石の中心を軸にして回す。
         *       当たり判定は持たない。
         */
        class Core : public Actor
        {
        public:
            Core();
            virtual ~Core() override;


        protected:
            /** 初期化処理。モデルをロードして、ワールド行列を確定させる */
            virtual void Start() override;
            /** 更新処理。回転を進めて、モデルを更新する */
            virtual void Update() override;
            /** 描画処理。モデルを描画する */
            virtual void Render(RenderContext& rc) override;


        private:
            /** 現在の回転と座標をモデルへ反映して、ワールド行列を更新する */
            void UpdateModel();


        private:
            /** 現在の自転の角度(度)。0以上360未満 */
            float m_yawDeg;
        };
    } // namespace actor
} // namespace app
