/**
 * @file TimerManager.h
 * @brief タイマーの管理をするクラス
 * @details タイマー(Timer)を作って登録し、動作中のものを毎フレームまとめて進める。シングルトン。
 *          使う側はTimerを直接持たず、CreateTimer()で受け取ったハンドルで操作する。
 *          ハンドルは「リストの何番目か(下位16bit)」と「そのスロットを何回使ったか(上位16bit)」でできている。
 *          消えたタイマーのハンドルが、あとから作られた別のタイマーを指すことはない。
 *          例: const TimerHandle handle = TimerManager::Get().CreateTimer(3.0f, EnTimeUpAction::Release);
 *              TimerManager::Get().StartTimer(handle);
 *              if (TimerManager::Get().IsTimeUp(handle)) { ... }
 *          NOTE: 毎フレームの更新(Update())は、Application::PreUpdate()から呼ぶ。
 */
#pragma once
#include <cstdint>
#include <vector>

#include "Timer.h"
#include "TimerHandle.h"


namespace app
{
    /**
     * @brief タイマーを管理するクラス
     */
    class TimerManager : public Noncopyable
    {
    private:
        /**
         * @brief リストの1マス。タイマーと、その扱いに必要な情報をまとめたもの
         */
        struct TimerSlot
        {
            /** コンストラクタ */
            TimerSlot()
                : m_timer()
                , m_isRunning(false)
                , m_timeUpAction(EnTimeUpAction::Hold)
                , m_generation(0)
            {
            }


            /** タイマー。使われていないスロットは、ハンドルがINVALID_TIMER_HANDLE */
            Timer m_timer;
            /** 動作中か。使われていないスロットは必ずfalse */
            bool m_isRunning;
            /** 時間切れになったあとの動作 */
            EnTimeUpAction m_timeUpAction;
            /** このスロットを何回使ったか。解放するたびに増え、ハンドルの上位16bitになる */
            uint16_t m_generation;
        };


    private:
        TimerManager();
        ~TimerManager();


    public:
        /**
         * @brief インスタンスを取得する
         * @details 初めて呼んだときに作られ、プロセスの終了時に破棄される。生成・破棄の呼び出しは要らない。
         * @return インスタンス
         */
        static TimerManager& Get();


        /**
         * @brief 更新処理
         * @details g_gameTime->GetFrameDeltaTime()の時間だけ、動作中のタイマーを進める。
         *          時間切れになったタイマーは、動作が止まる。EnTimeUpAction::Releaseのものは、リストから破棄される。
         *          毎フレーム、Application::PreUpdate()から呼ぶこと。
         */
        void Update();


        /**
         * @brief タイマーを作って登録する
         * @details 作った直後は止まっている。StartTimer()を呼ぶと進み始める。
         * @param timeLimit 制限時間(秒)。0以下なら、始めたらすぐに時間切れになる。
         * @param timeUpAction 時間切れになったあとの動作
         * @return ハンドル。登録できなかった場合はINVALID_TIMER_HANDLE。(同時に約6万5千個までしか作れない。)
         */
        TimerHandle CreateTimer(const float timeLimit, const EnTimeUpAction timeUpAction);


        /**
         * @brief タイマーを始める(再開する)
         * @details 止めていたタイマーは、続きから進む。時間切れのタイマーは、頭から数え直して始める。
         *          無効なハンドルは何もしない。
         * @param handle タイマーのハンドル
         */
        void StartTimer(const TimerHandle handle);


        /**
         * @brief タイマーを止める(一時停止)
         * @details 登録は残る。StartTimer()で続きから進む。無効なハンドルは何もしない。
         * @param handle タイマーのハンドル
         */
        void StopTimer(const TimerHandle handle);


        /**
         * @brief タイマーを登録から外す
         * @details このハンドルは、以後は無効になる。無効なハンドルは何もしない。
         * @param handle タイマーのハンドル
         */
        void DestroyTimer(const TimerHandle handle);


        /**
         * @brief 全てのタイマーを登録から外す
         * @details 全てのハンドルが、以後は無効になる。シーンを切り替えるときなどに使う。
         */
        void DestroyAllTimer();


        /**
         * @brief 時間切れか
         * @details 無効なハンドルは、時間切れとして扱う。
         *          EnTimeUpAction::Releaseのタイマーは時間切れで破棄されるので、「見つからない = 時間切れ」と読める。
         *          (破棄済みのハンドルや、INVALID_TIMER_HANDLEも同じ。)
         * @param handle タイマーのハンドル
         * @return 時間切れ、または無効なハンドルならtrue
         */
        bool IsTimeUp(const TimerHandle handle) const;


        /**
         * @brief ハンドルからタイマーを取得する
         * @details 残り時間や進み具合を読むために使う。取得したポインタは、保持せずにその場で使うこと。
         *          (Releaseで破棄されたり、DestroyTimer()で外されたりすると、使えなくなる。)
         * @param handle タイマーのハンドル
         * @return タイマー。無効なハンドルならnullptr。
         */
        const Timer* FindTimer(const TimerHandle handle) const;


    private:
        /**
         * @brief ハンドルから、リストの位置を探す
         * @param handle タイマーのハンドル
         * @return リストの位置。無効なハンドルなら-1。
         */
        int FindSlotIndex(const TimerHandle handle) const;


        /**
         * @brief スロットを解放して、次に作るタイマーが使えるようにする
         * @details 世代を進めるので、このスロットを指していた古いハンドルは無効になる。
         * @param index リストの位置
         */
        void ReleaseSlot(const uint16_t index);


    private:
        /** タイマーのリスト。要素は消さない。(消すと後ろの位置がずれて、ハンドルが狂うため。) */
        std::vector<TimerSlot> m_slots;
        /** 使われていないスロットの位置 */
        std::vector<uint16_t> m_freeIndices;
    };
} // namespace app
