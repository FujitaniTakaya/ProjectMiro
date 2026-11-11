/**
 * @file ClipPlayer.h
 * @brief 1つのクリップを再生するクラスの宣言
 */
#pragma once
#include <cstdint>
#include <vector>

#include "AnimationPose.h"
#include "ClipSampler.h"


namespace nsBalloonEngine
{
    /**
     * @brief クリップの再生
     * @details 1つのクリップの、時刻・ループ・再生中フラグ・イベントの通知を受け持つ。
     *          クロスフェードやレイヤーの重みは知らない(AnimationLayer が組み合わせる)。
     *          イベントの発火済みフラグはこのクラスが持つので、クリップを共有しても、他の再生に影響しない。
     */
    class ClipPlayer
    {
    public:
        ClipPlayer();
        ~ClipPlayer() = default;


        /**
         * @brief 初期化
         * @note サンプラーの一覧は、このクラスより長く生きていること。
         * @param samplers クリップのサンプラー一覧。添字がクリップ番号
         */
        void Init(const std::vector<ClipSampler>* samplers);


        /**
         * @brief クリップ番号が再生できるものか
         * @param clipIndex クリップの番号
         * @return 再生できるなら true
         */
        bool CanPlay(const int clipIndex) const;


        /**
         * @brief クリップを最初から再生する
         * @details 時刻を0に戻し、イベントを再武装する。同じクリップでも巻き戻す。
         * @param clipIndex クリップの番号。CanPlay() で確かめてあること
         */
        void Play(const int clipIndex);


        /**
         * @brief クリップの割り当てを外す
         */
        void Clear();


        /**
         * @brief 更新
         * @details 時間を進め、イベントを通知し、ポーズをサンプリングして pose に書く。
         *          ループするクリップは、余った時間を持ち越して巻き戻す。ループしないクリップは、終端を過ぎたら再生中でなくなり、最後のポーズを保つ。
         * @param deltaTime 進める時間(秒)
         * @param interpolate キーフレーム間を補間するか
         * @param listeners イベントの通知先
         * @param[in,out] pose 書き込み先。ボーン数分あること
         */
        void Update(const float deltaTime, const bool interpolate, const std::vector<AnimationEventListener>& listeners, AnimationPose& pose);


        /**
         * @brief クリップが割り当てられているか
         * @return 割り当てられているなら true
         */
        bool HasClip() const;
        /**
         * @brief 割り当てられているクリップの番号を取得
         * @return クリップの番号。割り当てが無ければ -1
         */
        int GetClipIndex() const;
        /**
         * @brief クリップが再生中か
         * @details ループしないクリップは、最後まで再生すると false になる。
         * @return 再生中なら true
         */
        bool IsPlaying() const;
        /**
         * @brief クリップ内の時刻を取得
         * @details ループするクリップは、巻き戻した後の時刻になる。ループしないクリップは、終わっても進み続ける。
         * @return 時刻(秒)
         */
        float GetTime() const;


    private:
        /** イベントの発火済みフラグを全て下ろす */
        void RearmEvents();
        /** 時刻を過ぎたイベントを通知する */
        void InvokeEvents(const ClipSampler& sampler, const std::vector<AnimationEventListener>& listeners);


    private:
        /** クリップのサンプラー一覧 */
        const std::vector<ClipSampler>* m_samplers;
        /** イベントの発火済みフラグ。クリップのイベントと同じ並び */
        std::vector<uint8_t> m_eventInvoked;
        /** 再生中のクリップ番号。割り当てが無ければ -1 */
        int m_clipIndex;
        /** クリップ内の時刻(秒)。ループしない場合は終わっても進み続ける */
        float m_time;
        /** 再生中か */
        bool m_isPlaying;
    };
} // namespace nsBalloonEngine
