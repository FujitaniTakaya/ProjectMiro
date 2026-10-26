/**
 * @file StageField.h
 * @brief ステージのフィールド
 */
#pragma once
#include "Source/Actor/Actor.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ステージのフィールド
         * @details 地面や壁など、ステージの背景になるモデル(stage.tkm)を描画する。
         *          Start() を一度呼んでから、毎フレーム Update() と Render() を呼ぶ。
         * @note モデルは Assets/modelData/stage/field/stage.tkm(Z-up・アニメーション無し)。
         *       動かさないので、ワールド行列は Start() で一度だけ確定させる。Start() の後に SetPosition() などをしても反映されない。
         *       当たり判定は持たない。GetModelRender() のモデルから、使う側が作る。
         */
        class StageField : public Actor
        {
        public:
            StageField();
            virtual ~StageField() override;


        protected:
            /** 初期化処理。モデルをロードして、ワールド行列を確定させる */
            virtual void Start() override;
            /** 更新処理。動かさないので、何もしない */
            virtual void Update() override;
            /** 描画処理。モデルを描画する */
            virtual void Render(RenderContext& rc) override;
        };
    } // namespace actor
} // namespace app
