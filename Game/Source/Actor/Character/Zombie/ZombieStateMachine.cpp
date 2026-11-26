/**
 * @file ZombieStateMachine.cpp
 * @brief ゾンビのステートマシン
 */
#include "stdafx.h"

#include "ZombieStateMachine.h"

#include "Source/Actor/Character/CharacterIState.h"
#include "Source/Actor/Character/Miro/MiroParameter.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Util/RandomDevice.h"
#include "ZombieIState.h"
#include "ZombieParameter.h"


namespace app
{
    namespace actor
    {
        ZombieStateMachine::ZombieStateMachine(CharacterBase* ownerCharacter, const ZombieParameter& parameter)
            : CharacterStateMachine(ownerCharacter)
            , m_parameter(parameter)
            , m_playerParameter(ParamHolder::Get().GetParameter<MiroParameter>(EnParamID::Miro))
            , m_hasFlinchRequest(false)
            , m_speedVariation(util::RandomDevice::Random(-1.0f, 1.0f))
            , m_phaseOffset(util::RandomDevice::Random(0.0f, 1.0f))
        {
            // 待機(慣性を消すゾンビ用)・歩き・走り・攻撃・死亡を登録する
            AddState<ZombieIdleState>(this);
            AddState<ZombieWalkState>(this);
            AddState<ZombieRunState>(this);
            AddState<ZombieAttackState>(this);
            AddState<ZombieDeathState>(this);
        }


        float ZombieStateMachine::GetWalkSpeed() const
        {
            return m_playerParameter.runSpeed * m_parameter.walkSpeed * GetAnimationSpeedScale();
        }


        float ZombieStateMachine::GetRunSpeed() const
        {
            return m_playerParameter.runSpeed * m_parameter.runSpeed * GetAnimationSpeedScale();
        }


        float ZombieStateMachine::GetAttackAnimationSpeed() const
        {
            return m_parameter.attackAnimSpeed;
        }


        float ZombieStateMachine::GetAnimationSpeedScale() const
        {
            return 1.0f + m_speedVariation * m_parameter.animSpeedRange;
        }


        bool ZombieStateMachine::CanChangeAttackState() const
        {
            // 攻撃の動きが終わるまでは続ける(途中でボタンを離しても、動きは最後まで再生する)
            if (IsEqualCurrentState(ZombieAttackState::ID()) && IsPlayingAnimation())
            {
                return true;
            }

            // 攻撃ボタン(ActionA)を押している間
            return GetInput().IsPress(EnInputButton::ActionA);
        }


        bool ZombieStateMachine::IsDeathFinished() const
        {
            // 死亡ステートに入っていて、死亡アニメーションの再生が終わった
            return IsEqualCurrentState(ZombieDeathState::ID()) && !IsPlayingAnimation();
        }


        void ZombieStateMachine::Update()
        {
            // 下半身(Base)の再生速度は個体ごとに違う。レイヤーの速度は持続するので、ホットリロードにも追従するよう毎フレーム入れる(上半身の怯みは別レイヤー)
            SetLayerAnimationSpeed(EnAnimationLayer::Base, GetAnimationSpeedScale());
            CharacterStateMachine::Update();
            UpdateFlinch();
        }


        void ZombieStateMachine::UpdateFlinch()
        {
            // 攻撃(全身の動き)と死亡は、怯みを重ねない。重なっていたら外す
            if (IsEqualCurrentState(ZombieAttackState::ID()) || IsEqualCurrentState(ZombieDeathState::ID()))
            {
                m_hasFlinchRequest = false;
                StopLayerAnimation(EnAnimationLayer::Upper, m_parameter.hitBlendTime);
                return;
            }

            if (m_hasFlinchRequest)
            {
                m_hasFlinchRequest = false;
                // 再生中なら巻き戻して最初から(フェードしない)。休止中なら、ブレンドして入る
                PlayLayerAnimation(EnAnimationLayer::Upper, EnZombieAnimationClipID::Hit, m_parameter.hitBlendTime, IsPlayingLayerAnimation(EnAnimationLayer::Upper));
                SetLayerAnimationSpeed(EnAnimationLayer::Upper, m_parameter.hitAnimSpeed);
                return;
            }

            // 終わったら外す。(ループしないクリップは、最後のポーズで重みが残るため。止まっているレイヤーに呼んでも何も起きない)
            if (!IsPlayingLayerAnimation(EnAnimationLayer::Upper))
            {
                StopLayerAnimation(EnAnimationLayer::Upper, m_parameter.hitBlendTime);
            }
        }


        IState* ZombieStateMachine::GetChangeState()
        {
            // 0. 死亡していれば、死亡へ。(一度入ったら抜けない。HPが回復しても生き返らない)
            if (IsEqualCurrentState(ZombieDeathState::ID()) || m_ownerCharacter->IsDead())
            {
                return FindState(ZombieDeathState::ID());
            }

            // 1. 攻撃に入る(続ける)なら、攻撃へ
            if (CanChangeAttackState())
            {
                return FindState(ZombieAttackState::ID());
            }

            // 2. 走りに切り替えられるなら走りへ
            if (CanChangeRunState())
            {
                return FindState(ZombieRunState::ID());
            }

            // 3. 移動入力があるなら歩きへ
            if (CanChangeMoveState())
            {
                return FindState(ZombieWalkState::ID());
            }

            // どれにも当てはまらなければ待機へ
            return FindState(ZombieIdleState::ID());
        }
    } // namespace actor
} // namespace app
