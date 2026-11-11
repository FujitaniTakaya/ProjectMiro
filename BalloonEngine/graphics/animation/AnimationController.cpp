/**
 * @file AnimationController.cpp
 * @brief アニメーション制御クラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "AnimationController.h"


namespace nsBalloonEngine
{
    AnimationController::AnimationController()
        : m_skeleton(nullptr)
        , m_samplers()
        , m_bindPose()
        , m_finalPose()
        , m_parentIndices()
        , m_modelMatrices()
        , m_layers()
        , m_listeners()
        , m_settings()
        , m_isInited(false)
    {}


    void AnimationController::Init(Skeleton& skeleton, AnimationClip* clips, const int clipNum)
    {
        m_skeleton = &skeleton;
        const int boneCount = skeleton.GetNumBones();

        //========================================================================
        // バインドポーズ(親ローカル)と、親のボーン番号を取り出す
        //========================================================================
        m_bindPose.resize(boneCount);
        m_parentIndices.resize(boneCount);
        for (int i = 0; i < boneCount; ++i)
        {
            const Bone* bone = skeleton.GetBone(i);
            m_bindPose[i] = animationpose::Decompose(bone->GetLocalMatrix());
            m_parentIndices[i] = bone->GetParentBoneNo();
            if (m_parentIndices[i] >= i)
            {
                K2_LOG("AnimationController: bone %d has a parent (%d) that does not come first. The pose will be wrong.\n", i, m_parentIndices[i]);
            }
        }
        m_finalPose = m_bindPose;

        //========================================================================
        // クリップを展開する
        //========================================================================
        m_samplers.resize(clipNum);
        for (int i = 0; i < clipNum; ++i)
        {
            m_samplers[i].Init(clips[i], boneCount);
        }

        //========================================================================
        // レイヤー
        //========================================================================
        for (size_t i = 0; i < m_layers.size(); ++i)
        {
            m_layers[i].Init(&m_samplers, &m_bindPose, i == static_cast<size_t>(EnAnimationLayer::Base));
        }

        m_isInited = true;

        // 従来の Animation と同じく、最初はクリップ 0 を再生しておく
        if (clipNum > 0)
        {
            Play(EnAnimationLayer::Base, 0, 0.0f, false);
        }
    }


    void AnimationController::Update(const float deltaTime)
    {
        if (!m_isInited)
        {
            return;
        }

        for (auto& layer : m_layers)
        {
            layer.Update(deltaTime, m_settings, m_listeners);
        }

        //========================================================================
        // Base の上に、Upper をボーンマスクの重みで重ねる
        //========================================================================
        const AnimationLayer& baseLayer = m_layers[static_cast<size_t>(EnAnimationLayer::Base)];
        m_finalPose = baseLayer.IsActive() ? baseLayer.GetPose() : m_bindPose;

        const AnimationLayer& upperLayer = m_layers[static_cast<size_t>(EnAnimationLayer::Upper)];
        if (upperLayer.IsActive())
        {
            animationpose::OverlayPose(m_finalPose, upperLayer.GetPose(), upperLayer.GetMask(), upperLayer.GetBlendWeight(m_settings));
        }

        //========================================================================
        // モデル空間の行列にして、スケルトンへ書き込む
        //========================================================================
        animationpose::ToModelSpace(m_finalPose, m_parentIndices, m_modelMatrices);
        const int boneCount = static_cast<int>(m_modelMatrices.size());
        for (int i = 0; i < boneCount; ++i)
        {
            m_skeleton->SetBoneLocalMatrix(i, m_modelMatrices[i]);
        }
        m_skeleton->SetMarkPlayAnimation();
    }


    void AnimationController::Play(const EnAnimationLayer layer, const int clipIndex, const float fadeTime, const bool isReplay)
    {
        if (!m_isInited || layer >= EnAnimationLayer::Max)
        {
            return;
        }
        m_layers[static_cast<size_t>(layer)].Play(clipIndex, fadeTime, isReplay);
    }


    void AnimationController::Stop(const EnAnimationLayer layer, const float fadeTime)
    {
        if (!m_isInited || layer >= EnAnimationLayer::Max)
        {
            return;
        }
        m_layers[static_cast<size_t>(layer)].Stop(fadeTime);
    }


    bool AnimationController::IsPlaying(const EnAnimationLayer layer) const
    {
        if (!m_isInited || layer >= EnAnimationLayer::Max)
        {
            return false;
        }
        return m_layers[static_cast<size_t>(layer)].IsPlaying();
    }


    bool AnimationController::SetLayerMask(const EnAnimationLayer layer, const wchar_t* rootBoneName)
    {
        if (!m_isInited || layer == EnAnimationLayer::Base || layer >= EnAnimationLayer::Max)
        {
            return false;
        }
        const int rootBone = m_skeleton->FindBoneID(rootBoneName);
        if (rootBone < 0)
        {
            K2_LOG("AnimationController: the mask bone was not found: %ls\n", rootBoneName);
            return false;
        }

        // 親をたどって、根のボーンに着くボーン(=サブツリー)だけを 1 にする。並び順に依存しない
        const int boneCount = static_cast<int>(m_parentIndices.size());
        std::vector<float> mask(boneCount, 0.0f);
        for (int i = 0; i < boneCount; ++i)
        {
            for (int bone = i; bone >= 0; bone = m_parentIndices[bone])
            {
                if (bone == rootBone)
                {
                    mask[i] = 1.0f;
                    break;
                }
            }
        }
        m_layers[static_cast<size_t>(layer)].SetMask(mask);
        return true;
    }


    void AnimationController::SetLayerSpeed(const EnAnimationLayer layer, const float speed)
    {
        if (!m_isInited || layer >= EnAnimationLayer::Max)
        {
            return;
        }
        m_layers[static_cast<size_t>(layer)].SetSpeed(speed);
    }


    void AnimationController::AddEventListener(const AnimationEventListener& listener)
    {
        m_listeners.push_back(listener);
    }


    void AnimationController::SetKeyframeInterpolation(const bool isEnabled)
    {
        m_settings.m_interpolateKeyframes = isEnabled;
    }


    void AnimationController::SetFadeEasing(const bool isEnabled)
    {
        m_settings.m_easeFade = isEnabled;
    }
} // namespace nsBalloonEngine
