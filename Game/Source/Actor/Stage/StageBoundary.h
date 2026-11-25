/**
 * @file StageBoundary.h
 * @brief ステージの外周の見えない壁
 */
#pragma once


namespace app
{
    namespace actor
    {
        /**
         * @brief ステージの外周の見えない壁
         * @details 壁専用のモデル(stage_wall.tkm)から、静的な当たり判定を作る。描画はしない(Draw() を呼ばない)。
         *          キャラクターコントローラーも弾も、この壁に当たる。Build() を一度呼んでから使う。
         *          破棄すると、物理世界から外れる。
         * @note 壁の大きさは、ステージ(stage.tkm)と同じ値で3ds Maxで作る。ステージの大きさを変えたら、壁も作り直す。
         */
        class StageBoundary
        {
        public:
            StageBoundary();
            ~StageBoundary();

            /**
             * @brief 壁の当たり判定を作る
             * @details 壁のモデルを読み、ワールド行列を確定させて、メッシュコライダーを作る。
             */
            void Build();


        private:
            /** 壁のモデル。当たり判定の形を取るためだけに読む。描画はしない */
            ModelRender m_modelRender;
            /** 壁の当たり判定 */
            PhysicsStaticObject m_physicsStaticObject;
        };
    } // namespace actor
} // namespace app
