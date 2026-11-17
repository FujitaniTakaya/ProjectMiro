/**
 * @file StartCountdown.h
 * @brief ゲーム開始前のカウントダウンの時間管理
 * @details 待ち時間 → count から1まで interval 秒ごとに数字を出す → 終了(0)、の順に進む。表示は持たない。
 *          経過時間は Timer が数える。このクラスは、経過時間から「今の数字」を求める。
 *          例: StartCountdown countdown(1.0f, 3, 1.0f);
 *              countdown.Update(deltaTime);
 *              if (countdown.IsCounting()) { 数字を表示(countdown.GetNumber(), countdown.GetNumberElapsed()); }
 *              if (countdown.IsFinished()) { ゲームを始める }
 */
#pragma once
#include <cstdint>

#include "Source/Core/Timer/Timer.h"


namespace app
{
    /**
     * @brief ゲーム開始前のカウントダウンの時間管理
     * @details 待ち時間の間と、終了した後は IsCounting() が false になる。終了した後は、Update() を呼んでも変わらない。
     */
    class StartCountdown : public Noncopyable
    {
    public:
        /**
         * @brief コンストラクタ
         * @param startDelay 数え始めるまでの待ち時間(秒)
         * @param count 最初に出す数(3なら 3,2,1 の順)
         * @param interval 数字を切り替える間隔(秒)。0以下だと、待ち時間が過ぎたらすぐに終了する
         */
        StartCountdown(const float startDelay, const int32_t count, const float interval);


    public:
        /**
         * @brief 時間を進める
         * @param deltaTime 進める時間(秒)
         */
        void Update(const float deltaTime);

        /**
         * @brief 数字を出している間かどうか
         * @return 数字を出している間はtrue。待ち時間の間と、終了した後はfalse
         */
        bool IsCounting() const;

        /**
         * @brief 0になったかどうか
         * @return 0になった(ゲームを始める)ならtrue
         */
        bool IsFinished() const
        {
            return m_timer.IsTimeUp();
        }

        /**
         * @brief 今の数字を取得
         * @return 今の数字(3,2,1)。IsCounting() の間だけ意味を持つ
         */
        int32_t GetNumber() const;

        /**
         * @brief 今の数字を出してからの経過時間を取得
         * @details 数字が出る時の動きに使う。
         * @return 経過時間(秒)。0〜interval。IsCounting() の間だけ意味を持つ
         */
        float GetNumberElapsed() const;


    private:
        /** 数え始めるまでの待ち時間(秒) */
        float m_startDelay;
        /** 最初に出す数 */
        int32_t m_count;
        /** 数字を切り替える間隔(秒) */
        float m_interval;
        /** 経過時間を数えるタイマー。制限時間は startDelay + count * interval */
        Timer m_timer;
    };
} // namespace app
