/**
 * @file MiroIState.h
 * @brief Miro専用のステート
 * @details Miroは移動方向と関係なく、カメラの方向を向いたまま動く。(ストレイフ)
 *          歩き・走りは、向いている方向から見た移動方向(8方向)で、再生するアニメーションを切り替える。
 *          構えは、ZLを押している間の専用ステート。ハンドガンを構えたポーズを保ち、歩きより遅く動ける。離すと、構えを解くステートが1回動いてから戻る。
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
        /** 前方宣言 */
        class MiroStateMachine;


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




        /*************************************************************/


        /**
         * @brief 構えステート
         * @details ZLを押している間のステート。待機から構えたポーズまでのアニメーション(GunShot01)を1回再生して、最後のポーズを保つ。(ループしないクリップは終端で止まる)
         *          構えながら動くアニメーションがまだないので、動くと構えのポーズのまま滑る。移動速度は、歩きより遅い構え中の速度(MiroParameter の aimMoveSpeed)。
         *          向きは、カメラの方向から少し中央側へ回した方向。(MiroStateMachine::UpdateRotation)
         *          構えたポーズを保っている間に、ZRを押した瞬間に1発撃つ。撃てたら、撃つ動き(GunShot02)を最初から再生する。
         *          構える動きと撃つ動きの間は撃てない。(撃つ動きの長さが、撃てる間隔になる)押した瞬間だけ撃つので、押しっぱなしでは連射しない。
         *          ZLを離すと、構えを解くステート(MiroAimEndState)へ移る。
         */
        class MiroAimState : public CharacterIState
        {
            appState(MiroAimState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            /**
             * @brief コンストラクタ
             * @param owner ステートを持つステートマシン。構え中の移動速度と、撃つ処理のために、Miroのステートマシンを受け取る
             */
            explicit MiroAimState(MiroStateMachine* owner);
            ~MiroAimState() override = default;


        private:
            /** 構え中の移動速度を読み、撃つ処理を呼ぶための、Miroのステートマシン */
            const MiroStateMachine* m_miroOwner;
        };




        /*************************************************************/


        /**
         * @brief 構えを解くステート
         * @details ZLを離した直後のステート。構えたポーズから待機のポーズまでのアニメーション(GunShot03)を1回再生する。
         *          動きが終わるまでは立ち止まる。終わると、入力に合わせて待機・歩き・走りへ移る。(遷移は MiroStateMachine)
         *          動いている間にZLを押し直すと、構えステートに戻る。
         */
        class MiroAimEndState : public CharacterIState
        {
            appState(MiroAimEndState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            explicit MiroAimEndState(CharacterStateMachine* owner);
            ~MiroAimEndState() override = default;
        };
    } // namespace actor
} // namespace app
