/**
 * @file MiroIState.h
 * @brief Miro専用のステート
 * @details Miroは移動方向と関係なく、カメラの方向を向いたまま動く。(ストレイフ)
 *          歩き・走りは、向いている方向から見た移動方向(8方向)で、再生するアニメーションを切り替える。
 *          構えは、ZLを押している間の専用ステート。ハンドガンを構えたポーズを保ち、歩きより遅く動ける。離すと、構えを解くステートが1回動いてから戻る。
 *          リロードは、Xを押した時とマガジンが0になった時のステート。リロードのアニメーションが終わるまで続く。
 *          構え・構えを解く・リロードは、上半身(Upper レイヤー)が銃の動き、下半身(Base レイヤー)が歩き・待機のアニメーション。(MiroStateMachine::PlayLowerBodyAnimation())
 *          上半身のレイヤーは、銃を扱わなくなった時に MiroStateMachine::Update() が止める。
 *          上半身は、下半身の歩きクリップが腰を斜めにねじっても、常に正面(アクターの向き)を保つ。(Miro::Start の SetLayerMask の keepRootOrientation)
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
            /**
             * @brief コンストラクタ
             * @param owner ステートを持つステートマシン。移動方向を求めるために、Miroのステートマシンを受け取る
             */
            explicit MiroMoveState(MiroStateMachine* owner);
            ~MiroMoveState() override = default;


        private:
            /** 移動方向に合うクリップを再生する。同じ族のクリップから切り替える時は、再生位置を引き継ぐ(MiroStateMachine::PlayLocomotionClip()) */
            void PlayDirectionalAnimation();


        private:
            /** 移動方向を求め、歩き・走りのクリップを再生するための、Miroのステートマシン */
            MiroStateMachine* m_miroOwner;
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
            explicit MiroWalkState(MiroStateMachine* owner);
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
            explicit MiroRunState(MiroStateMachine* owner);
            ~MiroRunState() override = default;
        };




        /*************************************************************/


        /**
         * @brief 構えステート
         * @details ZLを押している間のステート。上半身(Upper レイヤー)で、待機から構えたポーズまでのアニメーション(GunShot01)を1回再生して、最後のポーズを保つ。(ループしないクリップは終端で止まる)
         *          下半身(Base レイヤー)は、動いていれば歩き、止まっていれば待機。移動速度は、歩きより遅い構え中の速度(MiroParameter の aimMoveSpeed)で、
         *          歩きクリップも、その速度の比だけ遅く再生する。
         *          向きは、カメラの方向から少し中央側へ回した方向。(MiroStateMachine::UpdateRotation)
         *          構えたポーズを保っている間に、ZRを押した瞬間に1発撃つ。撃てたら、上半身で撃つ動き(GunShot02)を最初から再生する。下半身は歩き続ける。
         *          構える動きと撃つ動きの間は撃てない。(撃つ動きの長さが、撃てる間隔になる)押した瞬間だけ撃つので、押しっぱなしでは連射しない。
         *          撃つ動きの間は、リロードもできない。(IsFiring())構える動きの間は、リロードできる。
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

            /**
             * @brief 撃つ動きの最中かどうか
             * @return 撃って、撃つ動き(GunShot02)がまだ終わっていなければ true
             */
            bool IsFiring() const;


        public:
            /**
             * @brief コンストラクタ
             * @param owner ステートを持つステートマシン。構え中の移動速度と、撃つ処理のために、Miroのステートマシンを受け取る
             */
            explicit MiroAimState(MiroStateMachine* owner);
            ~MiroAimState() override = default;


        private:
            /** 構え中の移動速度を読み、下半身のアニメーションを再生し、撃つ処理を呼ぶための、Miroのステートマシン */
            MiroStateMachine* m_miroOwner;
            /** 撃つ動きの最中か */
            bool m_isFiring;
        };




        /*************************************************************/


        /**
         * @brief 構えを解くステート
         * @details ZLを離した直後のステート。上半身(Upper レイヤー)で、構えたポーズから待機のポーズまでのアニメーション(GunShot03)を1回再生する。
         *          立ち止まらず、構え中と同じ速度(aimMoveSpeed)で動き続けられる。下半身(Base レイヤー)は、動いていれば歩き、止まっていれば待機。
         *          終わると、入力に合わせて待機・歩き・走りへ移る。(遷移は MiroStateMachine)
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
            /**
             * @brief コンストラクタ
             * @param owner ステートを持つステートマシン。移動速度と下半身のアニメーションのために、Miroのステートマシンを受け取る
             */
            explicit MiroAimEndState(MiroStateMachine* owner);
            ~MiroAimEndState() override = default;


        private:
            /** 構え中の移動速度を読み、下半身のアニメーションを再生するための、Miroのステートマシン */
            MiroStateMachine* m_miroOwner;
        };




        /*************************************************************/


        /**
         * @brief リロードステート
         * @details 上半身(Upper レイヤー)で、リロードのアニメーション(PistolReload。2.03秒)を1回再生して、武器のリロードを始める。
         *          Xを押した時(マガジンが満タンでなく、撃つ動きの最中でない時)と、マガジンが0になった時に入る。(遷移は MiroStateMachine)
         *          アニメーションが終わると、入力に合わせて構え・待機・歩き・走りへ移る。
         *          下半身(Base レイヤー)は、動いていれば歩き、止まっていれば待機。構え中と同じ、歩きより遅い速度(aimMoveSpeed)で動ける。
         *          アニメーションは手前の構えのポーズから始まる。伸ばした腕の構えのポーズとは違うので、構え中に入ると、ブレンドでポーズが変わる。
         */
        class MiroReloadState : public CharacterIState
        {
            appState(MiroReloadState);

        public:
            // IStateの仮想関数のオーバーライド
            void Enter() override final;
            void Update() override final;
            void Exit() override final;


        public:
            /**
             * @brief コンストラクタ
             * @param owner ステートを持つステートマシン。リロードを始める処理と、移動速度のために、Miroのステートマシンを受け取る
             */
            explicit MiroReloadState(MiroStateMachine* owner);
            ~MiroReloadState() override = default;


        private:
            /** リロード中の移動速度を読み、下半身のアニメーションを再生し、リロードを始める処理を呼ぶための、Miroのステートマシン */
            MiroStateMachine* m_miroOwner;
        };
    } // namespace actor
} // namespace app
