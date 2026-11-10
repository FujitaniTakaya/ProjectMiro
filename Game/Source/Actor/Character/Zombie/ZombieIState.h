/**
 * @file ZombieIState.h
 * @brief ゾンビ専用のステート
 * @details ゾンビは進行方向を向いて動く(CharacterStateMachine の既定の向き)ので、歩き・走りのアニメーションは前進の1本ずつ。
 *          攻撃は、その場で止まって行う。
 *          待機は、キャラクター共通の待機(CharacterIdleState)に、入るときに慣性を消す処理を足した ZombieIdleState を使う。
 *          アニメーションは EnZombieAnimationClipID の番号で再生する。
 */
#pragma once
#include "Source/Actor/Character/CharacterIState.h"
#include "ZombieAnimationClipID.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 待機ステート
         * @details キャラクター共通の待機に、入るときに水平方向の速度を0にする処理を足したもの。
         *          コアに着いて入力がなくなったとき、摩擦による減速(慣性)で滑らずに、その場で止まる。
         */
        class ZombieIdleState : public CharacterIdleState
        {
            appState(ZombieIdleState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;


        public:
            explicit ZombieIdleState(CharacterStateMachine* owner);
            ~ZombieIdleState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 動くステートの基底
         * @details 歩きと走りで共通の処理。入るときにクリップを再生して、毎フレーム移動速度を設定して移動する。
         *          派生クラスは、移動速度と、再生するクリップを返す。
         */
        class ZombieMoveState : public CharacterIState
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
             * @brief 再生するアニメーションクリップを取得する
             * @return アニメーションクリップID
             */
            virtual EnZombieAnimationClipID GetClip() const = 0;


        protected:
            explicit ZombieMoveState(CharacterStateMachine* owner);
            ~ZombieMoveState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 歩きステート
         */
        class ZombieWalkState : public ZombieMoveState
        {
            appState(ZombieWalkState);

        protected:
            float GetMoveSpeed() const override final;
            EnZombieAnimationClipID GetClip() const override final;


        public:
            explicit ZombieWalkState(CharacterStateMachine* owner);
            ~ZombieWalkState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 走りステート
         */
        class ZombieRunState : public ZombieMoveState
        {
            appState(ZombieRunState);

        protected:
            float GetMoveSpeed() const override final;
            EnZombieAnimationClipID GetClip() const override final;


        public:
            explicit ZombieRunState(CharacterStateMachine* owner);
            ~ZombieRunState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 攻撃ステート
         * @details その場で攻撃のアニメーションを再生する。ActionA(攻撃)を押し続けている間は、終わるたびに最初から再生し直す。
         *          移動入力は使わないので、向きは変わらない。(コアへ向かってきた向きのまま攻撃する)
         * @note ダメージを与える処理は、まだ無い。
         */
        class ZombieAttackState : public CharacterIState
        {
            appState(ZombieAttackState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            explicit ZombieAttackState(CharacterStateMachine* owner);
            ~ZombieAttackState() override = default;
        };
    } // namespace actor
} // namespace app
