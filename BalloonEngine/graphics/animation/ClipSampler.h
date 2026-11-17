/**
 * @file ClipSampler.h
 * @brief アニメーションクリップをTRSに展開して、時刻からポーズを取り出すクラスの宣言
 */
#pragma once
#include <string>
#include <vector>

#include "AnimationPose.h"


namespace nsBalloonEngine
{
    /**
     * @brief クリップのサンプラー
     * @details クリップのキーフレーム(親ローカル行列)を、初期化時に位置・回転・拡大へ展開しておく。
     *          Sample() は、前後のキーフレームを補間してポーズを書き込む。
     *          クリップ自体(AnimationClip)は読み取るだけで、イベントの発火済みフラグには触らない。
     */
    class ClipSampler
    {
    public:
        /** クリップが持つアニメーションイベント */
        struct Event
        {
            /** 発火する時刻(秒) */
            float m_invokeTime;
            /** イベント名 */
            std::wstring m_name;


            Event()
                : m_invokeTime(0.0f)
                , m_name()
            {}
        };


    public:
        ClipSampler();
        ~ClipSampler() = default;


        /**
         * @brief 初期化
         * @details キーを持つボーンだけを保持する。最上位のキー列(最初にキーを持つボーン)の時刻を、全ボーンの時刻として使う。
         * @note クリップは、このサンプラーより長く生きていること。
         * @param clip 展開するクリップ。ロード済みであること
         * @param boneCount スケルトンのボーン数。これ以上の番号のキーは無視する
         */
        void Init(AnimationClip& clip, const int boneCount);


        /**
         * @brief ポーズを取り出す
         * @details キーを持つボーンだけを pose に書く。キーの無いボーンは触らない(呼び出し側がバインドポーズで初期化しておく)。
         *          time が最後のキーフレームより後なら、最後のキーフレームで止める。
         * @param time クリップ内の時刻(秒)
         * @param[in,out] pose 書き込み先。ボーン数分あること
         * @param interpolate true なら前後のキーフレームを補間する。false なら time 以上の最初のキーフレームをそのまま使う(k2EngineLow の Animation と同じ)
         */
        void Sample(const float time, AnimationPose& pose, const bool interpolate) const;


        /**
         * @brief クリップの長さ(最後のキーフレームの時刻)を取得
         * @return 長さ(秒)。キーが無ければ 0
         */
        float GetDuration() const;
        /**
         * @brief キーフレーム数を取得
         * @return キーフレーム数
         */
        int GetFrameCount() const;
        /**
         * @brief ループするかどうか
         * @details クリップのループフラグを、その都度読む。
         * @return ループするなら true
         */
        bool IsLoop() const;
        /**
         * @brief 全ボーンのキー数が同じかどうか
         * @details 違うボーンは、足りない分を最後のキーで埋めて展開している。
         * @return 同じなら true
         */
        bool HasUniformKeyCount() const;
        /**
         * @brief クリップの名前を取得
         * @return クリップ名。イベント通知に使う
         */
        const wchar_t* GetClipName() const;
        /**
         * @brief アニメーションイベントの一覧を取得
         * @return イベントの一覧
         */
        const std::vector<Event>& GetEvents() const;


    private:
        /** 元のクリップ */
        const AnimationClip* m_clip;
        /** キーフレームの時刻(秒) */
        std::vector<float> m_frameTimes;
        /** キーを持つボーンの番号 */
        std::vector<int> m_keyedBones;
        /** 展開したトランスフォーム。[フレーム * キーを持つボーン数 + キーを持つボーンの順番] */
        std::vector<BoneTransform> m_frames;
        /** アニメーションイベント */
        std::vector<Event> m_events;
        /** クリップ名 */
        std::wstring m_clipName;
        /** 全ボーンのキー数が同じか */
        bool m_hasUniformKeyCount;
    };
} // namespace nsBalloonEngine
