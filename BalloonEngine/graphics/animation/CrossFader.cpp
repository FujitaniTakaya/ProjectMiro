/**
 * @file CrossFader.cpp
 * @brief ポーズのクロスフェードの実装
 */
#include "BalloonEnginePreCompile.h"

#include "CrossFader.h"

#include <algorithm>


namespace nsBalloonEngine
{
    CrossFader::CrossFader()
        : m_fromPose()
        , m_elapsed(0.0f)
        , m_duration(0.0f)
        , m_isFading(false)
    {}


    void CrossFader::Start(const AnimationPose& fromPose, const float duration)
    {
        if (duration <= 0.0f)
        {
            Cancel();
            return;
        }
        m_fromPose = fromPose;
        m_elapsed = 0.0f;
        m_duration = duration;
        m_isFading = true;
    }


    void CrossFader::Cancel()
    {
        m_isFading = false;
    }


    void CrossFader::Apply(AnimationPose& pose, const float deltaTime, const bool ease)
    {
        if (!m_isFading)
        {
            return;
        }

        m_elapsed += deltaTime;
        const float t = (std::min)(m_elapsed / m_duration, 1.0f);
        if (t >= 1.0f)
        {
            m_isFading = false;
            return;
        }

        const float weight = ease ? animationpose::SmoothStep(t) : t;
        animationpose::BlendPose(pose, m_fromPose, pose, weight);
    }


    bool CrossFader::IsFading() const
    {
        return m_isFading;
    }
} // namespace nsBalloonEngine
