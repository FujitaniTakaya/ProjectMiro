/**
 * @file AnimationLayer.cpp
 * @brief 1レイヤー分のアニメーションの実装
 */
#include "BalloonEnginePreCompile.h"

#include "AnimationLayer.h"

#include <algorithm>


namespace nsBalloonEngine
{
    AnimationLayer::AnimationLayer()
        : m_bindPose(nullptr)
        , m_player()
        , m_fader()
        , m_pose()
        , m_mask()
        , m_maskRootBone(-1)
        , m_keepRootOrientation(false)
        , m_followBone(-1)
        , m_followRatio(0.0f)
        , m_speed(1.0f)
        , m_isBase(false)
        , m_weight(0.0f)
        , m_targetWeight(0.0f)
        , m_weightSpeed(0.0f)
    {}


    void AnimationLayer::Init(const std::vector<ClipSampler>* samplers, const AnimationPose* bindPose, const bool isBase)
    {
        m_bindPose = bindPose;
        m_isBase = isBase;
        m_player.Init(samplers);
        m_fader.Cancel();
        m_pose = *bindPose;
        // 初期のマスクは全ボーン 1(レイヤーが全身に効く)
        m_mask.assign(bindPose->size(), 1.0f);
        m_maskRootBone = -1;
        m_keepRootOrientation = false;
        m_speed = 1.0f;
        m_weight = isBase ? 1.0f : 0.0f;
        m_targetWeight = m_weight;
        m_weightSpeed = 0.0f;
    }


    void AnimationLayer::Play(const int clipIndex, const float fadeTime, const bool isReplay)
    {
        if (!m_player.CanPlay(clipIndex))
        {
            return;
        }
        // 同じクリップを再生中なら何もしない(止めている最中のレイヤーは、再開させる)
        if (clipIndex == m_player.GetClipIndex() && !isReplay && m_targetWeight > 0.0f)
        {
            return;
        }

        const bool wasActive = m_player.HasClip();
        const float fade = isReplay ? 0.0f : (std::max)(fadeTime, 0.0f);

        //========================================================================
        // クロスフェード: 切り替える瞬間の出力を保存して、そこから新しいクリップへ向かう
        //========================================================================
        if (wasActive && m_targetWeight > 0.0f)
        {
            m_fader.Start(m_pose, fade);
        }
        else
        {
            m_fader.Cancel();
        }

        m_player.Play(clipIndex);
        // キーの無いボーンに前のクリップの値が残らないよう、バインドポーズから始める
        m_pose = *m_bindPose;

        //========================================================================
        // ベース以外は、休止中(または止めている最中)ならレイヤーの重みを 1 へ向かわせる
        //========================================================================
        if (!m_isBase && (!wasActive || m_targetWeight <= 0.0f))
        {
            if (!wasActive)
            {
                m_weight = 0.0f;
            }
            SetTargetWeight(1.0f, fade);
        }
    }


    void AnimationLayer::Stop(const float fadeTime)
    {
        if (m_isBase || !m_player.HasClip())
        {
            return;
        }
        SetTargetWeight(0.0f, (std::max)(fadeTime, 0.0f));
        if (m_weight <= 0.0f)
        {
            Deactivate();
        }
    }


    void AnimationLayer::Update(const float deltaTime, const AnimationBlendSettings& settings, const std::vector<AnimationEventListener>& listeners)
    {
        if (!m_player.HasClip())
        {
            return;
        }

        //========================================================================
        // レイヤーの重みを進める。0 になったら休止する
        //========================================================================
        if (!m_isBase && m_weight != m_targetWeight)
        {
            const float step = m_weightSpeed * deltaTime;
            if (m_weight < m_targetWeight)
            {
                m_weight = (std::min)(m_weight + step, m_targetWeight);
            }
            else
            {
                m_weight = (std::max)(m_weight - step, m_targetWeight);
            }
        }
        if (!m_isBase && m_weight <= 0.0f && m_targetWeight <= 0.0f)
        {
            Deactivate();
            return;
        }

        //========================================================================
        // クリップを進めてサンプリングし、クロスフェードをかける
        // (再生速度はクリップの時間にだけ掛ける。フェードは実時間)
        //========================================================================
        m_player.Update(deltaTime * m_speed, settings.m_interpolateKeyframes, listeners, m_pose);
        m_fader.Apply(m_pose, deltaTime, settings.m_easeFade);
    }


    void AnimationLayer::SetMask(const std::vector<float>& weights, const int rootBone, const bool keepRootOrientation)
    {
        m_mask = weights;
        m_maskRootBone = rootBone;
        m_keepRootOrientation = keepRootOrientation;
    }


    void AnimationLayer::SetFollowBone(const int bone)
    {
        m_followBone = bone;
    }


    void AnimationLayer::SetFollowRatio(const float ratio)
    {
        m_followRatio = std::clamp(ratio, 0.0f, 1.0f);
    }


    void AnimationLayer::SetSpeed(const float speed)
    {
        m_speed = speed;
    }


    bool AnimationLayer::IsActive() const
    {
        return m_player.HasClip();
    }


    bool AnimationLayer::IsPlaying() const
    {
        return m_player.IsPlaying() && (m_isBase || m_targetWeight > 0.0f);
    }


    float AnimationLayer::GetBlendWeight(const AnimationBlendSettings& settings) const
    {
        if (m_isBase)
        {
            return 1.0f;
        }
        return settings.m_easeFade ? animationpose::SmoothStep(m_weight) : m_weight;
    }


    const AnimationPose& AnimationLayer::GetPose() const
    {
        return m_pose;
    }


    const std::vector<float>& AnimationLayer::GetMask() const
    {
        return m_mask;
    }


    int AnimationLayer::GetMaskRootBone() const
    {
        return m_maskRootBone;
    }


    bool AnimationLayer::KeepsRootOrientation() const
    {
        return m_keepRootOrientation;
    }


    int AnimationLayer::GetFollowBone() const
    {
        return m_followBone;
    }


    float AnimationLayer::GetFollowRatio() const
    {
        return m_followRatio;
    }


    void AnimationLayer::SetTargetWeight(const float target, const float duration)
    {
        m_targetWeight = target;
        if (duration <= 0.0f)
        {
            m_weight = target;
            m_weightSpeed = 0.0f;
        }
        else
        {
            m_weightSpeed = 1.0f / duration;
        }
    }


    void AnimationLayer::Deactivate()
    {
        m_player.Clear();
        m_fader.Cancel();
    }
} // namespace nsBalloonEngine
