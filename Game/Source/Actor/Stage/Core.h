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
         *          座標・回転・拡大率は SetPosition() などで設定する。
         *          Start() を一度呼んでから、毎フレーム Update() と Render() を呼ぶ。
         * @note モデルは Assets/modelData/stage/core/Gemme.tkm(Z-up・アニメーション無し)。
         *       モデルはメートル単位で書き出されている(高さ1.32)。ワールドはcm単位なので、拡大率の初期値は100倍にしてある。
         *       モデルの原点は、ほぼ中心にある。
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
            /** 更新処理。モデルを更新する */
            virtual void Update() override;
            /** 描画処理。モデルを描画する */
            virtual void Render(RenderContext& rc) override;
        };
    } // namespace actor
} // namespace app
