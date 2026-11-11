/**
 * @file CrossFader.h
 * @brief ポーズのクロスフェードを行うクラスの宣言
 */
#pragma once
#include "AnimationPose.h"


namespace nsBalloonEngine
{
    /**
     * @brief クロスフェード
     * @details 切り替えた瞬間のポーズを保存しておき、新しいポーズへ、時間をかけてブレンドしていく。
     *          クリップやレイヤーは知らない。保存したポーズから始めるので、フェードの途中で
     *          もう一度 Start() しても、見た目が飛ばない。フェードの長さと回数に上限は無い。
     */
    class CrossFader
    {
    public:
        CrossFader();
        ~CrossFader() = default;


        /**
         * @brief フェードを始める
         * @details すでにフェード中なら、そのポーズ(フェード元)に置き換えて最初からやり直す。
         * @param fromPose フェード元のポーズ(切り替える直前の出力)
         * @param duration フェードの時間(秒)。0以下なら、フェードしない
         */
        void Start(const AnimationPose& fromPose, const float duration);


        /**
         * @brief フェードを止める
         */
        void Cancel();


        /**
         * @brief フェードを進めて、ポーズに反映する
         * @details フェード元と pose をブレンドして、結果を pose に書く。終わったら何もしなくなる。
         * @param[in,out] pose 新しいポーズ。結果がここに入る
         * @param deltaTime 進める時間(秒)
         * @param ease true なら smoothstep をかける。false なら線形
         */
        void Apply(AnimationPose& pose, const float deltaTime, const bool ease);


        /**
         * @brief フェード中か
         * @return フェード中なら true
         */
        bool IsFading() const;


    private:
        /** フェード元のポーズ */
        AnimationPose m_fromPose;
        /** 経過時間(秒) */
        float m_elapsed;
        /** フェードの長さ(秒) */
        float m_duration;
        /** フェード中か */
        bool m_isFading;
    };
} // namespace nsBalloonEngine
