/**
 * @file CharacterIState.h
 * @brief キャラクター共通のステート
 * @details Miroとゾンビなど、全キャラクターで共通のステートをまとめる。
 *          ステートの処理はキャラクターで共通にして、違いはデータ(アニメーションの番号、移動速度)で出す。
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




        /*************************************************************/


        /**
         * @brief 歩きステート
         */
        class CharacterWalkState : public CharacterIState
        {
            appState(CharacterWalkState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            explicit CharacterWalkState(CharacterStateMachine* owner);
            ~CharacterWalkState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 走りステート
         */
        class CharacterRunState : public CharacterIState
        {
            appState(CharacterRunState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            explicit CharacterRunState(CharacterStateMachine* owner);
            ~CharacterRunState() override = default;
        };
    } // namespace actor
} // namespace app
