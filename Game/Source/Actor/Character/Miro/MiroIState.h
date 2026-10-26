/**
 * @file MiroIState.h
 * @brief Miro専用のステート
 * @details Miroは移動方向と関係なく、カメラの方向を向いたまま動く。(ストレイフ)
 *          歩き・走りは、向いている方向から見た移動方向(8方向)で、再生するアニメーションを切り替える。
 *          待機は、キャラクター共通のステート(CharacterIdleState)を使う。
 */
#pragma once
#include "MiroAnimationClipID.h"
#include "MiroMoveDirection.h"
#include "Source/Actor/Character/CharacterIState.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 8方向に動くステートの基底
         * @details 歩きと走りで共通の処理。毎フレーム、移動方向から再生するクリップを選び、移動する。
         *          同じクリップの再生は何もしないので、毎フレーム呼んでよい。方向が変わったときだけ、標準のブレンドで切り替わる。
         *          派生クラスは、移動速度と、方向ごとのクリップを返す。
         */
        class MiroMoveState : public CharacterIState
        {
        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        protected:
            /**
             * @brief 移動速度を取得する
             * @return 入力の倒し具合が最大のときの速さ
             */
            virtual float GetMoveSpeed() const = 0;
            /**
             * @brief 方向に合うアニメーションクリップを取得する
             * @param direction 向いている方向から見た移動方向
             * @return アニメーションクリップID
             */
            virtual EnMiroAnimationClipID GetClip(const EnMiroMoveDirection direction) const = 0;


        protected:
            explicit MiroMoveState(CharacterStateMachine* owner);
            ~MiroMoveState() override = default;


        private:
            /** 移動方向に合うクリップを再生する */
            void PlayDirectionalAnimation();
        };




        /*************************************************************/


        /**
         * @brief 歩きステート(8方向)
         */
        class MiroWalkState : public MiroMoveState
        {
            appState(MiroWalkState);

        protected:
            float GetMoveSpeed() const override final;
            EnMiroAnimationClipID GetClip(const EnMiroMoveDirection direction) const override final;


        public:
            explicit MiroWalkState(CharacterStateMachine* owner);
            ~MiroWalkState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 走りステート(8方向)
         */
        class MiroRunState : public MiroMoveState
        {
            appState(MiroRunState);

        protected:
            float GetMoveSpeed() const override final;
            EnMiroAnimationClipID GetClip(const EnMiroMoveDirection direction) const override final;


        public:
            explicit MiroRunState(CharacterStateMachine* owner);
            ~MiroRunState() override = default;
        };
    } // namespace actor
} // namespace app
