/**
 * @file Timer.h
 * @brief 制限時間を数えるクラス
 * @details 制限時間をセットして、Update()で経過時間を進める。エンジンには依存しない。
 *          複数のタイマーをまとめて進めたいときは、TimerManagerに作ってもらう。
 */
#pragma once
#include "TimerHandle.h"


namespace app
{
    class TimerManager;


    /**
     * @brief 制限時間を数えるクラス
     * @details 経過時間は0〜制限時間の範囲で、制限時間を超えては進まない。
     *          SetTimeLimit()をする前は制限時間が0なので、時間切れとして扱われる。
     */
    class Timer
    {
    public:
        /**
         * @brief コンストラクタ
         * @details 制限時間0、経過0、ハンドルなし(INVALID_TIMER_HANDLE)で作られる。
         */
        Timer();


    public:
        /**
         * @brief 制限時間をセットして、数え直す
         * @param seconds 制限時間(秒)。0以下なら0になり、すぐに時間切れになる。
         */
        void SetTimeLimit(const float seconds);


        /**
         * @brief 時間を進める
         * @param deltaTime 進める時間(秒)。0以下とNaNは何もしない。
         */
        void Update(const float deltaTime);


        /**
         * @brief 経過時間を0に戻す
         * @details 制限時間はそのまま。
         */
        void Reset();


        /**
         * @brief 時間切れか
         * @return 経過時間が制限時間に達していればtrue
         */
        bool IsTimeUp() const
        {
            return m_elapsed >= m_timeLimit;
        }


        /**
         * @brief 制限時間を取得する
         * @return 制限時間(秒)
         */
        float GetTimeLimit() const
        {
            return m_timeLimit;
        }


        /**
         * @brief 経過時間を取得する
         * @return 経過時間(秒)。0〜制限時間。
         */
        float GetElapsedTime() const
        {
            return m_elapsed;
        }


        /**
         * @brief 残り時間を取得する
         * @return 残り時間(秒)。0〜制限時間。負にはならない。
         */
        float GetRemainingTime() const
        {
            return m_timeLimit - m_elapsed;
        }


        /**
         * @brief 進み具合を取得する
         * @return 0.0(始まったところ)〜1.0(時間切れ)。制限時間が0なら1.0。
         */
        float GetProgress() const;


        /**
         * @brief このタイマーのハンドルを取得する
         * @details TimerManagerが作ったタイマーだけが持つ。単体で使っているタイマーはINVALID_TIMER_HANDLE。
         *          TimerManagerから解除されたタイマーは、INVALID_TIMER_HANDLEに戻る。
         * @return ハンドル
         */
        TimerHandle GetHandle() const
        {
            return m_handle;
        }


    private:
        /** ハンドルを書き込めるのは、タイマーを登録するTimerManagerだけ */
        friend class TimerManager;


        /**
         * @brief ハンドルをセットする
         * @param handle ハンドル
         */
        void SetHandle(const TimerHandle handle)
        {
            m_handle = handle;
        }


    private:
        /** 制限時間(秒) */
        float m_timeLimit;
        /** 経過時間(秒)。0〜制限時間 */
        float m_elapsed;
        /** このタイマーのハンドル */
        TimerHandle m_handle;
    };
} // namespace app
