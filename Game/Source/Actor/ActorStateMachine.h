/**
 * @file ActorStateMachine.h
 * @brief アクターのステートマシン
 */
#pragma once
#include <cstdint>

#include "Actor.h"
#include "Source/MoveBehavior/StateMachineBase.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief アクターのステートマシン
         * @details アクター共通の処理(アニメーションの再生、トランスフォームの参照)をまとめる。
         *          トランスフォームは持たない。持ち主の Actor::m_transform が唯一の正で、ここから変えるときは持ち主の SetPosition() などを呼ぶ。
         *          持ち主に求めるのは Actor の公開機能だけなので、Actor の派生ならどれにでも付けられる。
         */
        class ActorStateMachine : public StateMachineBase
        {
        public:
            /**
             * アニメーション切り替え時の標準ブレンド時間(秒)
             * 0だと前のポーズから瞬間的に切り替わってカクつくため、全キャラ共通で短いクロスフェードを挟む
             */
            static constexpr float ANIMATION_INTERPOLATE_TIME = 0.15f;


        public:
            /**
             * @brief アニメーションを再生する
             * @tparam TEnum アニメーションIDの型
             * @param animationID アニメーションID。アニメーションクリップの並びの番号になる
             * @param interpolateTime ブレンド時間(秒)。省略時は標準ブレンド
             */
            template <typename TEnum>
            inline void PlayAnimation(const TEnum animationID, const float interpolateTime = ANIMATION_INTERPOLATE_TIME)
            {
                m_ownerActor->GetModelRender().PlayAnimation(static_cast<uint8_t>(animationID), interpolateTime);
            }
            /**
             * @brief アニメーションの再生速度を設定する
             * @param speed 再生速度(1.0f=通常速度)
             */
            inline void SetAnimationSpeed(const float speed)
            {
                m_ownerActor->SetAnimationSpeed(speed);
            }
            /**
             * @brief アニメーションが再生中かどうか
             * @return 再生中なら true
             */
            inline bool IsPlayingAnimation() const
            {
                return m_ownerActor->GetModelRender().IsPlayingAnimation();
            }
            /**
             * @brief トランスフォームを取得する
             * @return 持ち主のトランスフォームの参照
             */
            inline const Transform& GetTransform() const
            {
                return m_ownerActor->GetTransform();
            }


        protected:
            /**
             * @brief ステートの変更先を取得する
             * @details 何もしない(遷移しない)。派生クラスでオーバーライドして遷移を決める。
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;


        public:
            /**
             * @brief コンストラクタ
             * @param ownerActor 持ち主のアクター
             */
            explicit ActorStateMachine(Actor* ownerActor);
            virtual ~ActorStateMachine() override = default;


        protected:
            /** 持ち主のアクター */
            Actor* m_ownerActor;
        };
    } // namespace actor
} // namespace app
