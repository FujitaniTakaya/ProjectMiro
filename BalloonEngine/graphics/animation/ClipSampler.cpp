/**
 * @file ClipSampler.cpp
 * @brief アニメーションクリップのサンプラーの実装
 */
#include "BalloonEnginePreCompile.h"

#include "ClipSampler.h"

#include <algorithm>


namespace nsBalloonEngine
{
    ClipSampler::ClipSampler()
        : m_clip(nullptr)
        , m_frameTimes()
        , m_keyedBones()
        , m_frames()
        , m_events()
        , m_clipName()
        , m_hasUniformKeyCount(true)
    {}


    void ClipSampler::Init(AnimationClip& clip, const int boneCount)
    {
        m_clip = &clip;
        m_clipName = clip.GetName();
        m_frameTimes.clear();
        m_keyedBones.clear();
        m_frames.clear();
        m_events.clear();
        m_hasUniformKeyCount = true;

        //========================================================================
        // イベントをコピーする(発火済みフラグはクリップではなく再生側が持つ)
        //========================================================================
        const int eventNum = clip.GetNumAnimationEvent();
        auto& animationEvents = clip.GetAnimationEvent();
        if (eventNum > 0 && animationEvents)
        {
            m_events.resize(eventNum);
            for (int i = 0; i < eventNum; ++i)
            {
                m_events[i].m_invokeTime = animationEvents[i].GetInvokeTime();
                m_events[i].m_name = animationEvents[i].GetEventName();
            }
        }

        //========================================================================
        // キーを持つボーンを集める。最初に見つかったボーンの時刻を、全ボーンの時刻とする
        //========================================================================
        const auto& keyFrameLists = clip.GetKeyFramePtrListArray();
        const int listNum = (std::min)(static_cast<int>(keyFrameLists.size()), boneCount);
        int topBone = -1;
        for (int bone = 0; bone < listNum; ++bone)
        {
            if (keyFrameLists[bone].empty())
            {
                continue;
            }
            if (topBone < 0)
            {
                topBone = bone;
            }
            m_keyedBones.push_back(bone);
        }
        if (topBone < 0)
        {
            return;
        }

        const auto& topList = keyFrameLists[topBone];
        const size_t frameCount = topList.size();
        m_frameTimes.resize(frameCount);
        for (size_t frame = 0; frame < frameCount; ++frame)
        {
            m_frameTimes[frame] = topList[frame]->time;
        }

        //========================================================================
        // 行列→TRS に展開する
        //========================================================================
        const size_t keyedBoneNum = m_keyedBones.size();
        m_frames.resize(frameCount * keyedBoneNum);
        for (size_t k = 0; k < keyedBoneNum; ++k)
        {
            const auto& list = keyFrameLists[m_keyedBones[k]];
            if (list.size() != frameCount)
            {
                m_hasUniformKeyCount = false;
                K2_LOG("ClipSampler: the key count of bone %d differs from the top bone (%d / %d)\n", m_keyedBones[k], static_cast<int>(list.size()), static_cast<int>(frameCount));
            }
            for (size_t frame = 0; frame < frameCount; ++frame)
            {
                // 足りないキーは最後のキーで埋める
                const size_t source = (std::min)(frame, list.size() - 1);
                m_frames[frame * keyedBoneNum + k] = animationpose::Decompose(list[source]->transform);
            }
        }
    }


    void ClipSampler::Sample(const float time, AnimationPose& pose, const bool interpolate) const
    {
        if (m_frameTimes.empty())
        {
            return;
        }

        const int lastFrame = static_cast<int>(m_frameTimes.size()) - 1;

        // time 以上の最初のキーフレーム
        int next = static_cast<int>(std::lower_bound(m_frameTimes.begin(), m_frameTimes.end(), time) - m_frameTimes.begin());
        int previous = next;
        float fraction = 0.0f;
        if (next > lastFrame)
        {
            // 終端より後は、最後のキーフレームで止める
            next = lastFrame;
            previous = lastFrame;
        }
        else if (interpolate && next > 0)
        {
            previous = next - 1;
            const float span = m_frameTimes[next] - m_frameTimes[previous];
            fraction = (span > 0.0f) ? ((time - m_frameTimes[previous]) / span) : 1.0f;
        }

        const size_t keyedBoneNum = m_keyedBones.size();
        const BoneTransform* previousFrame = &m_frames[previous * keyedBoneNum];
        const BoneTransform* nextFrame = &m_frames[next * keyedBoneNum];
        for (size_t k = 0; k < keyedBoneNum; ++k)
        {
            pose[m_keyedBones[k]] = (previous == next)
                                        ? nextFrame[k]
                                        : animationpose::Blend(previousFrame[k], nextFrame[k], fraction);
        }
    }


    float ClipSampler::GetDuration() const
    {
        return m_frameTimes.empty() ? 0.0f : m_frameTimes.back();
    }


    int ClipSampler::GetFrameCount() const
    {
        return static_cast<int>(m_frameTimes.size());
    }


    bool ClipSampler::IsLoop() const
    {
        return m_clip != nullptr && m_clip->IsLoop();
    }


    bool ClipSampler::HasUniformKeyCount() const
    {
        return m_hasUniformKeyCount;
    }


    const wchar_t* ClipSampler::GetClipName() const
    {
        return m_clipName.c_str();
    }


    const std::vector<ClipSampler::Event>& ClipSampler::GetEvents() const
    {
        return m_events;
    }
} // namespace nsBalloonEngine
