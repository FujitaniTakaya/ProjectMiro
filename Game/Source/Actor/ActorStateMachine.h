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
             * @param isReplay 今のクリップと同じでも、最初から巻き戻して再生する。(連続して撃つ動きなど)巻き戻す時は、ブレンドしない
             */
            template <typename TEnum>
            inline void PlayAnimation(const TEnum animationID, const float interpolateTime = ANIMATION_INTERPOLATE_TIME, const bool isReplay = false)
            {
                m_ownerActor->GetModelRender().PlayAnimation(static_cast<uint8_t>(animationID), interpolateTime, isReplay);
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
             * @brief レイヤーを指定してアニメーションを再生する
             * @details 上半身と下半身で別のクリップを再生する時に使う。Upper レイヤーは、モデルのボーンマスクの範囲だけを、Base(PlayAnimation())の上に重ねる。
             * @tparam TEnum アニメーションIDの型
             * @param layer 再生するレイヤー
             * @param animationID アニメーションID。アニメーションクリップの並びの番号になる
             * @param interpolateTime ブレンド時間(秒)。省略時は標準ブレンド。休止中の Upper では、レイヤーのフェードインの時間にもなる
             * @param isReplay 今のクリップと同じでも、最初から巻き戻して再生する。巻き戻す時は、ブレンドしない
             */
            template <typename TEnum>
            inline void PlayLayerAnimation(const EnAnimationLayer layer, const TEnum animationID, const float interpolateTime = ANIMATION_INTERPOLATE_TIME, const bool isReplay = false)
            {
                m_ownerActor->GetModelRender().PlayLayerAnimation(layer, static_cast<uint8_t>(animationID), interpolateTime, isReplay);
            }
            /**
             * @brief レイヤーのアニメーションを止める
             * @details フェードアウトして、見えなくなったら休止する。Base は止められない。止まっているレイヤーに呼んでも何も起きない。
             * @param layer 止めるレイヤー
             * @param fadeTime フェードアウトの時間(秒)。省略時は標準ブレンド
             */
            inline void StopLayerAnimation(const EnAnimationLayer layer, const float fadeTime = ANIMATION_INTERPOLATE_TIME)
            {
                m_ownerActor->GetModelRender().StopLayerAnimation(layer, fadeTime);
            }
            /**
             * @brief レイヤーのアニメーションが再生中かどうか
             * @details ループしないクリップは、最後まで再生すると false になる。
             * @param layer 調べるレイヤー
             * @return 再生中なら true
             */
            inline bool IsPlayingLayerAnimation(const EnAnimationLayer layer) const
            {
                return m_ownerActor->GetModelRender().IsPlayingLayerAnimation(layer);
            }
            /**
             * @brief レイヤーのアニメーション再生速度を設定する
             * @details そのレイヤーのクリップだけに効く。ブレンドの長さは変わらない。
             * @param layer 設定するレイヤー
             * @param speed 再生速度(1.0f=通常速度)
             */
            inline void SetLayerAnimationSpeed(const EnAnimationLayer layer, const float speed)
            {
                m_ownerActor->GetModelRender().SetLayerAnimationSpeed(layer, speed);
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
