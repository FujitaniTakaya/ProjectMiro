/**
 * @file TimerManager.cpp
 * @brief タイマーの管理をするクラス
 */
#include "stdafx.h"

#include "TimerManager.h"


namespace app
{
    namespace
    {
        /** ハンドルの下位に入れる、リストの位置のビット数 */
        constexpr uint32_t INDEX_BITS = 16;
        /** リストの位置を取り出すマスク */
        constexpr uint32_t INDEX_MASK = (1u << INDEX_BITS) - 1;
        /** 作れるスロットの数。位置0xFFFFは使わない。(INVALID_TIMER_HANDLEの位置と同じになってしまうため。) */
        constexpr size_t MAX_SLOT_COUNT = INDEX_MASK;
        /** 見つからなかったときの位置 */
        constexpr int INVALID_INDEX = -1;


        /**
         * @brief リストの位置と世代から、ハンドルを作る
         * @param index リストの位置
         * @param generation 世代
         * @return ハンドル
         */
        TimerHandle MakeHandle(const uint16_t index, const uint16_t generation)
        {
            return (static_cast<TimerHandle>(generation) << INDEX_BITS) | static_cast<TimerHandle>(index);
        }


        /**
         * @brief ハンドルから、リストの位置を取り出す
         * @param handle ハンドル
         * @return リストの位置
         */
        uint32_t HandleToIndex(const TimerHandle handle)
        {
            return handle & INDEX_MASK;
        }
    } // namespace


    TimerManager::TimerManager()
        : m_slots()
        , m_freeIndices()
    {
    }


    TimerManager::~TimerManager()
    {
        // NOTE: プロセスの終了時に破棄される。エンジンには触らない。
    }


    TimerManager& TimerManager::Get()
    {
        static TimerManager instance;
        return instance;
    }


    void TimerManager::Update()
    {
        // 進める時間。1/60fの固定値ではなく、エンジンの値を使う。(固定フレームレートなら1/maxFPS、可変なら計測値。)
        const float deltaTime = g_gameTime->GetFrameDeltaTime();

        // NOTE: 位置で回すので、途中でスロットを解放しても安全。
        for (size_t i = 0; i < m_slots.size(); ++i)
        {
            TimerSlot& slot = m_slots[i];
            if (!slot.m_isRunning)
            {
                continue;
            }

            slot.m_timer.Update(deltaTime);
            if (!slot.m_timer.IsTimeUp())
            {
                continue;
            }

            // 時間切れ。動作は止める。
            slot.m_isRunning = false;
            if (slot.m_timeUpAction == EnTimeUpAction::Release)
            {
                ReleaseSlot(static_cast<uint16_t>(i));
            }
        }
    }


    TimerHandle TimerManager::CreateTimer(const float timeLimit, const EnTimeUpAction timeUpAction)
    {
        uint16_t index = 0;
        if (!m_freeIndices.empty())
        {
            // 使われていないスロットを再利用する。
            index = m_freeIndices.back();
            m_freeIndices.pop_back();
        }
        else
        {
            // NOTE: そんなに同時に作るはずがない。
            if (m_slots.size() >= MAX_SLOT_COUNT)
            {
                K2_ASSERT(false, "タイマーの数が多すぎます。\n");
                return INVALID_TIMER_HANDLE;
            }
            index = static_cast<uint16_t>(m_slots.size());
            m_slots.emplace_back();
        }

        TimerSlot& slot = m_slots[index];
        slot.m_timer.SetTimeLimit(timeLimit);
        slot.m_isRunning = false;
        slot.m_timeUpAction = timeUpAction;

        const TimerHandle handle = MakeHandle(index, slot.m_generation);
        slot.m_timer.SetHandle(handle);
        return handle;
    }


    void TimerManager::StartTimer(const TimerHandle handle)
    {
        const int index = FindSlotIndex(handle);
        if (index == INVALID_INDEX)
        {
            return;
        }

        TimerSlot& slot = m_slots[index];
        // 時間切れのタイマーは、頭から数え直す。(Holdで残したタイマーを、使い回せる。)
        if (slot.m_timer.IsTimeUp())
        {
            slot.m_timer.Reset();
        }
        slot.m_isRunning = true;
    }


    void TimerManager::StopTimer(const TimerHandle handle)
    {
        const int index = FindSlotIndex(handle);
        if (index == INVALID_INDEX)
        {
            return;
        }
        m_slots[index].m_isRunning = false;
    }


    void TimerManager::DestroyTimer(const TimerHandle handle)
    {
        const int index = FindSlotIndex(handle);
        if (index == INVALID_INDEX)
        {
            return;
        }
        ReleaseSlot(static_cast<uint16_t>(index));
    }


    void TimerManager::DestroyAllTimer()
    {
        for (size_t i = 0; i < m_slots.size(); ++i)
        {
            // 使われていないスロットは、すでに解放されている。(二重に空きリストへ入れない。)
            if (m_slots[i].m_timer.GetHandle() != INVALID_TIMER_HANDLE)
            {
                ReleaseSlot(static_cast<uint16_t>(i));
            }
        }
    }


    bool TimerManager::IsTimeUp(const TimerHandle handle) const
    {
        const Timer* timer = FindTimer(handle);
        // 見つからないタイマーは、Releaseで破棄されたものとして、時間切れ扱いにする。
        return timer == nullptr || timer->IsTimeUp();
    }


    const Timer* TimerManager::FindTimer(const TimerHandle handle) const
    {
        const int index = FindSlotIndex(handle);
        if (index == INVALID_INDEX)
        {
            return nullptr;
        }
        return &m_slots[index].m_timer;
    }


    int TimerManager::FindSlotIndex(const TimerHandle handle) const
    {
        // NOTE: INVALID_TIMER_HANDLEの位置は0xFFFF。スロットは0xFFFF個未満なので、ここで弾かれる。
        const uint32_t index = HandleToIndex(handle);
        if (index >= m_slots.size())
        {
            return INVALID_INDEX;
        }

        // 世代が古いハンドルや、解放済みのスロットは、タイマーが持つハンドルと一致しない。
        if (m_slots[index].m_timer.GetHandle() != handle)
        {
            return INVALID_INDEX;
        }
        return static_cast<int>(index);
    }


    void TimerManager::ReleaseSlot(const uint16_t index)
    {
        TimerSlot& slot = m_slots[index];
        slot.m_timer.SetHandle(INVALID_TIMER_HANDLE);
        slot.m_isRunning = false;
        // 世代を進めて、このスロットを指していた古いハンドルを無効にする。(16bitを超えたら0に戻る。)
        slot.m_generation = static_cast<uint16_t>(slot.m_generation + 1);
        m_freeIndices.push_back(index);
    }
} // namespace app
