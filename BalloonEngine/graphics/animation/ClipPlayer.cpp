/**
 * @file ClipPlayer.cpp
 * @brief 1つのクリップを再生するクラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "ClipPlayer.h"

#include <algorithm>
#include <cmath>


namespace nsBalloonEngine
{
    ClipPlayer::ClipPlayer()
        : m_samplers(nullptr)
        , m_eventInvoked()
        , m_clipIndex(-1)
        , m_time(0.0f)
        , m_isPlaying(false)
    {}


    void ClipPlayer::Init(const std::vector<ClipSampler>* samplers)
    {
        m_samplers = samplers;
        Clear();
    }


    bool ClipPlayer::CanPlay(const int clipIndex) const
    {
        return m_samplers != nullptr && clipIndex >= 0 && clipIndex < static_cast<int>(m_samplers->size());
    }


    void ClipPlayer::Play(const int clipIndex, const float startTime)
    {
        m_clipIndex = clipIndex;
        m_time = (std::max)(startTime, 0.0f);
        m_isPlaying = true;
        RearmEvents();

        // 開始より前のイベントは鳴らさない。(Update() は m_time > 発火時刻 で通知するので、同じ比較で発火済みにする)
        const auto& events = (*m_samplers)[m_clipIndex].GetEvents();
        for (size_t i = 0; i < events.size(); ++i)
        {
            m_eventInvoked[i] = (m_time > events[i].m_invokeTime) ? 1 : 0;
        }
    }


    void ClipPlayer::Clear()
    {
        m_clipIndex = -1;
        m_time = 0.0f;
        m_isPlaying = false;
        m_eventInvoked.clear();
    }


    void ClipPlayer::Update(const float deltaTime, const bool interpolate, const std::vector<AnimationEventListener>& listeners, AnimationPose& pose)
    {
        if (m_clipIndex < 0)
        {
            return;
        }

        const ClipSampler& sampler = (*m_samplers)[m_clipIndex];
        const float duration = sampler.GetDuration();

        m_time += deltaTime;
        InvokeEvents(sampler, listeners);

        if (sampler.IsLoop())
        {
            if (duration > 0.0f && m_time > duration)
            {
                // 余った時間を持ち越して巻き戻す(ループのたびに時間が遅れない)
                m_time = std::fmod(m_time, duration);
                RearmEvents();
                InvokeEvents(sampler, listeners);
            }
        }
        else if (m_time > duration)
        {
            m_isPlaying = false;
        }

        sampler.Sample(m_time, pose, interpolate);
    }


    bool ClipPlayer::HasClip() const
    {
        return m_clipIndex >= 0;
    }


    int ClipPlayer::GetClipIndex() const
    {
        return m_clipIndex;
    }


    bool ClipPlayer::IsPlaying() const
    {
        return m_clipIndex >= 0 && m_isPlaying;
    }


    float ClipPlayer::GetTime() const
    {
        return m_time;
    }


    float ClipPlayer::GetPhase() const
    {
        if (m_clipIndex < 0)
        {
            return -1.0f;
        }
        const ClipSampler& sampler = (*m_samplers)[m_clipIndex];
        const float duration = sampler.GetDuration();
        if (!sampler.IsLoop() || duration <= 0.0f)
        {
            return -1.0f;
        }
        return std::fmod(m_time, duration) / duration;
    }


    bool ClipPlayer::IsLoop(const int clipIndex) const
    {
        return CanPlay(clipIndex) && (*m_samplers)[clipIndex].IsLoop();
    }


    float ClipPlayer::GetDuration(const int clipIndex) const
    {
        return CanPlay(clipIndex) ? (*m_samplers)[clipIndex].GetDuration() : 0.0f;
    }


    void ClipPlayer::RearmEvents()
    {
        if (m_samplers == nullptr || m_clipIndex < 0)
        {
            m_eventInvoked.clear();
            return;
        }
        m_eventInvoked.assign((*m_samplers)[m_clipIndex].GetEvents().size(), 0);
    }


    void ClipPlayer::InvokeEvents(const ClipSampler& sampler, const std::vector<AnimationEventListener>& listeners)
    {
        const auto& events = sampler.GetEvents();
        if (m_eventInvoked.size() != events.size())
        {
            m_eventInvoked.assign(events.size(), 0);
        }
        for (size_t i = 0; i < events.size(); ++i)
        {
            if (m_eventInvoked[i] == 0 && m_time > events[i].m_invokeTime)
            {
                m_eventInvoked[i] = 1;
                for (const auto& listener : listeners)
                {
                    listener(sampler.GetClipName(), events[i].m_name.c_str());
                }
            }
        }
    }
} // namespace nsBalloonEngine
