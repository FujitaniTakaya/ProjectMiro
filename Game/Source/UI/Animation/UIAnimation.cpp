/**
 * @file UIAnimation.cpp
 * @brief UIのパーツを動かすアニメーション
 */
#include "stdafx.h"

#include "UIAnimation.h"
#include "UIAnimationParameter.h"
#include "Source/UI/UIParts.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 定義から、始める値と終わる値を取り出す。(型ごと) */
            void ReadStartEnd(const UIAnimationDef& def, float& start, float& end)
            {
                start = def.startFloat;
                end = def.endFloat;
            }

            void ReadStartEnd(const UIAnimationDef& def, Vector2& start, Vector2& end)
            {
                start = def.startV2;
                end = def.endV2;
            }

            void ReadStartEnd(const UIAnimationDef& def, Vector3& start, Vector3& end)
            {
                start = def.startV3;
                end = def.endV3;
            }

            void ReadStartEnd(const UIAnimationDef& def, Vector4& start, Vector4& end)
            {
                start = def.startV4;
                end = def.endV4;
            }


            /** 型に対応する、定義の値の種類を取得する。(ポインタは型を渡すためだけに使う。) */
            UIAnimationDef::ValueType ValueTypeOf(const float*)
            {
                return UIAnimationDef::ValueType::Float;
            }

            UIAnimationDef::ValueType ValueTypeOf(const Vector2*)
            {
                return UIAnimationDef::ValueType::Vector2;
            }

            UIAnimationDef::ValueType ValueTypeOf(const Vector3*)
            {
                return UIAnimationDef::ValueType::Vector3;
            }

            UIAnimationDef::ValueType ValueTypeOf(const Vector4*)
            {
                return UIAnimationDef::ValueType::Vector4;
            }
        } // namespace




        //=======================================================================
        // UIAnimationBase
        //=======================================================================
        UIAnimationBase::UIAnimationBase()
            : m_ui(nullptr)
        {}


        UIAnimationBase::~UIAnimationBase()
        {}




        //=======================================================================
        // UIValueAnimation
        //=======================================================================
        template <typename T>
        UIValueAnimation<T>::UIValueAnimation()
            : m_curve()
            , m_applyFunc()
            , m_definitionKey(0)
            , m_definitionRevision(0)
        {}


        template <typename T>
        UIValueAnimation<T>::~UIValueAnimation()
        {}


        template <typename T>
        void UIValueAnimation<T>::Update()
        {
            if (!m_curve.IsPlaying())
            {
                return;
            }

            m_curve.Update(g_gameTime->GetFrameDeltaTime());
            // 終わったフレームも、終わった時の値を反映する。
            Apply(m_curve.GetCurrentValue());
        }


        template <typename T>
        void UIValueAnimation<T>::PlayAnimation()
        {
            RefreshDefinition();
            m_curve.Play();
        }


        template <typename T>
        void UIValueAnimation<T>::StopAnimation()
        {
            m_curve.Stop();
        }


        template <typename T>
        bool UIValueAnimation<T>::IsPlayAnimation() const
        {
            return m_curve.IsPlaying();
        }


        template <typename T>
        void UIValueAnimation<T>::SetParameter(
            const T& start,
            const T& end,
            const float timeSec,
            const util::EasingType type,
            const util::LoopMode loopMode,
            const uint16_t repeatCount
        )
        {
            m_curve.Initialize(start, end, timeSec, type, loopMode, repeatCount);
            // 値を設定したので、jsonの定義には従わない。
            m_definitionKey = 0;
        }


        template <typename T>
        void UIValueAnimation<T>::SetEndBehavior(const util::EndBehavior endBehavior)
        {
            m_curve.SetEndBehavior(endBehavior);
        }


        template <typename T>
        void UIValueAnimation<T>::SetDefinition(const UIAnimationDef& def)
        {
            if (def.valueType != ValueTypeOf(static_cast<const T*>(nullptr)))
            {
                K2_LOG("UIアニメーションの定義の\"valueType\"が、アニメーションの型と違います。key=%u\n", def.key);
            }

            ApplyDefinition(def);
            m_definitionKey = def.key;
            m_definitionRevision = UIAnimationParameter::Get().GetRevision();
        }


        template <typename T>
        T UIValueAnimation<T>::GetCurrentValue() const
        {
            return m_curve.GetCurrentValue();
        }


        template <typename T>
        void UIValueAnimation<T>::SetFunc(const UIAnimationApplyFunc<T>& func)
        {
            m_applyFunc = func;
        }


        template <typename T>
        void UIValueAnimation<T>::Apply(const T& value)
        {
            if (m_applyFunc)
            {
                m_applyFunc(value);
            }
        }


        template <typename T>
        void UIValueAnimation<T>::ApplyDefinition(const UIAnimationDef& def)
        {
            T start;
            T end;
            ReadStartEnd(def, start, end);
            m_curve.Initialize(start, end, def.duration, def.easingType, def.loopMode, def.repeatCount);
            m_curve.SetEndBehavior(def.endBehavior);
        }


        template <typename T>
        void UIValueAnimation<T>::RefreshDefinition()
        {
            if (m_definitionKey == 0)
            {
                return;
            }

            const UIAnimationParameter& parameter = UIAnimationParameter::Get();
            if (parameter.GetRevision() == m_definitionRevision)
            {
                return;
            }
            m_definitionRevision = parameter.GetRevision();

            // 定義がjsonから消えていた場合は、今の値のまま再生する。
            const UIAnimationDef* def = parameter.Find(m_definitionKey);
            if (def)
            {
                ApplyDefinition(*def);
            }
        }


        // テンプレートの全メンバーを、使われていなくてもここでコンパイルする。
        template class UIValueAnimation<float>;
        template class UIValueAnimation<Vector2>;
        template class UIValueAnimation<Vector3>;
        template class UIValueAnimation<Vector4>;




        //=======================================================================
        // UIColorAnimation
        //=======================================================================
        UIColorAnimation::UIColorAnimation()
        {}


        UIColorAnimation::~UIColorAnimation()
        {}


        void UIColorAnimation::Apply(const Vector4& value)
        {
            if (m_ui)
            {
                m_ui->m_color = value;
            }
        }




        //=======================================================================
        // UIScaleAnimation
        //=======================================================================
        UIScaleAnimation::UIScaleAnimation()
        {}


        UIScaleAnimation::~UIScaleAnimation()
        {}


        void UIScaleAnimation::Apply(const Vector3& value)
        {
            if (m_ui)
            {
                m_ui->m_transform.m_localTransform.m_scale = value;
            }
        }




        //=======================================================================
        // UITranslateAnimation
        //=======================================================================
        UITranslateAnimation::UITranslateAnimation()
        {}


        UITranslateAnimation::~UITranslateAnimation()
        {}


        void UITranslateAnimation::Apply(const Vector3& value)
        {
            if (m_ui)
            {
                m_ui->m_transform.m_localTransform.m_position = value;
            }
        }




        //=======================================================================
        // UIRotationAnimation
        //=======================================================================
        UIRotationAnimation::UIRotationAnimation()
        {}


        UIRotationAnimation::~UIRotationAnimation()
        {}


        void UIRotationAnimation::Apply(const float& value)
        {
            if (m_ui)
            {
                m_ui->m_transform.m_localTransform.m_rotation.SetRotationDegZ(value);
            }
        }




        //=======================================================================
        // UIAnimationStep
        //=======================================================================
        UIAnimationStep::UIAnimationStep(
            const uint32_t key,
            const float delay,
            std::function<void()> start,
            std::function<void()> complete
        )
            : animationKey(key)
            , delayBefore(delay)
            , onStart(std::move(start))
            , onComplete(std::move(complete))
        {}




        //=======================================================================
        // UIAnimationSequence
        //=======================================================================
        UIAnimationSequence::UIAnimationSequence()
            : m_steps()
            , m_currentIndex(-1)
            , m_isPlaying(false)
            , m_delayTimer(0.0f)
            , m_waitingDelay(false)
            , m_playId(0)
            , m_target(nullptr)
            , m_onSequenceComplete()
        {}


        UIAnimationSequence::~UIAnimationSequence()
        {}


        UIAnimationSequence& UIAnimationSequence::Add(const uint32_t animKey, const float delayBefore)
        {
            m_steps.emplace_back(animKey, delayBefore, nullptr, nullptr);
            return *this;
        }


        UIAnimationSequence& UIAnimationSequence::Add(
            const uint32_t animKey,
            const float delayBefore,
            std::function<void()> onStart,
            std::function<void()> onComplete
        )
        {
            m_steps.emplace_back(animKey, delayBefore, std::move(onStart), std::move(onComplete));
            return *this;
        }


        UIAnimationSequence& UIAnimationSequence::OnComplete(std::function<void()> callback)
        {
            m_onSequenceComplete = std::move(callback);
            return *this;
        }


        void UIAnimationSequence::Play(UIBase* target)
        {
            if (!target || m_steps.empty())
            {
                return;
            }

            m_target = target;
            m_currentIndex = -1;
            m_isPlaying = true;
            m_waitingDelay = false;
            m_delayTimer = 0.0f;
            ++m_playId;
            AdvanceToNext();
        }


        void UIAnimationSequence::Stop()
        {
            m_isPlaying = false;
            m_currentIndex = -1;
            m_waitingDelay = false;
            ++m_playId;
        }


        void UIAnimationSequence::Clear()
        {
            m_steps.clear();
            Stop();
        }


        void UIAnimationSequence::Update(const float deltaTime)
        {
            if (!m_isPlaying || !m_target)
            {
                return;
            }

            if (m_waitingDelay)
            {
                m_delayTimer -= deltaTime;
                if (m_delayTimer > 0.0f)
                {
                    return;
                }
                m_waitingDelay = false;
                StartCurrentStep();
                return;
            }

            if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_steps.size()))
            {
                return;
            }

            // 再生中のアニメーションが終わるのを待つ。(取り除かれた場合は、終わったものとして扱う。)
            const UIAnimationBase* animation = m_target->FindAnimation(m_steps[m_currentIndex].animationKey);
            if (animation && animation->IsPlayAnimation())
            {
                return;
            }

            // NOTE: 関数の中でClear()などが呼ばれると、ステップが破棄されるので、コピーして呼ぶ。
            const uint32_t playId = m_playId;
            if (m_steps[m_currentIndex].onComplete)
            {
                const std::function<void()> onComplete = m_steps[m_currentIndex].onComplete;
                onComplete();
                // 関数の中で、Play() Stop() Clear()された場合は、そちらを優先する。
                if (playId != m_playId)
                {
                    return;
                }
            }
            AdvanceToNext();
        }


        void UIAnimationSequence::AdvanceToNext()
        {
            ++m_currentIndex;
            if (m_currentIndex >= static_cast<int>(m_steps.size()))
            {
                // 全てのステップが終わった。
                m_isPlaying = false;
                if (m_onSequenceComplete)
                {
                    const std::function<void()> onComplete = m_onSequenceComplete;
                    onComplete();
                }
                return;
            }

            const float delayBefore = m_steps[m_currentIndex].delayBefore;
            if (delayBefore > 0.0f)
            {
                m_delayTimer = delayBefore;
                m_waitingDelay = true;
            }
            else
            {
                m_waitingDelay = false;
                StartCurrentStep();
            }
        }


        void UIAnimationSequence::StartCurrentStep()
        {
            if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_steps.size()))
            {
                return;
            }

            const uint32_t animationKey = m_steps[m_currentIndex].animationKey;
            if (!m_target->FindAnimation(animationKey))
            {
                // アニメーションが登録されていない場合は、飛ばす。
                K2_LOG("UIアニメーションが登録されていないステップを飛ばします。key=%u\n", animationKey);
                AdvanceToNext();
                return;
            }

            // NOTE: 関数の中でClear()などが呼ばれると、ステップが破棄されるので、コピーして呼ぶ。
            if (m_steps[m_currentIndex].onStart)
            {
                const uint32_t playId = m_playId;
                const std::function<void()> onStart = m_steps[m_currentIndex].onStart;
                onStart();
                if (playId != m_playId)
                {
                    return;
                }
            }

            // 関数の中で、アニメーションが取り除かれたかもしれないので、取り直す。
            UIAnimationBase* animation = m_target->FindAnimation(animationKey);
            if (animation)
            {
                animation->PlayAnimation();
            }
        }
    } // namespace ui
} // namespace app
