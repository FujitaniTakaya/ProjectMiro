/**
 * @file CharacterIState.h
 * @brief キャラクター共通のステート
 * @details Miroとゾンビなど、全キャラクターで共通のステートをまとめる。今は待機だけ。
 *          歩き・走りは、動き方(アニメーションの方向数、向きの決め方)がキャラクターで大きく違うので、キャラクターごとに持つ。
 *          (Miroは Miro/MiroIState.h)
 *          アニメーションは EnCharacterAnimationClipID の番号で再生する。
 */
#pragma once
#include "Source/MoveBehavior/StateMachineBase.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        class CharacterStateMachine;


        /**
         * @brief キャラクター共通のステートのインターフェース
         */
        class CharacterIState : public IState
        {
        public:
            virtual void Enter() override
            {}
            virtual void Update() override
            {}
            virtual void Exit() override
            {}


        public:
            /**
             * @brief コンストラクタ
             * @param owner ステートを持つステートマシン
             */
            explicit CharacterIState(CharacterStateMachine* owner);
            ~CharacterIState() override = default;


        protected:
            /** ステートのオーナー */
            CharacterStateMachine* m_owner;
        };




        /*************************************************************/


        /**
         * @brief 待機ステート
         */
        class CharacterIdleState : public CharacterIState
        {
            appState(CharacterIdleState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            explicit CharacterIdleState(CharacterStateMachine* owner);
            ~CharacterIdleState() override = default;
        };
    } // namespace actor
} // namespace app
