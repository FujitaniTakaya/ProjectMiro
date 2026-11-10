/**
 * @file ZombieStateMachine.cpp
 * @brief ゾンビのステートマシン
 */
#include "stdafx.h"

#include "ZombieStateMachine.h"

#include "Source/Actor/Character/CharacterIState.h"
#include "Source/Actor/Character/Miro/MiroParameter.h"
#include "Source/Core/Param/ParamHolder.h"
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
        {
            // 待機(慣性を消すゾンビ用)・歩き・走り・攻撃を登録する
            AddState<ZombieIdleState>(this);
            AddState<ZombieWalkState>(this);
            AddState<ZombieRunState>(this);
            AddState<ZombieAttackState>(this);
        }


        float ZombieStateMachine::GetWalkSpeed() const
        {
            return m_playerParameter.runSpeed * m_parameter.walkSpeed;
        }


        float ZombieStateMachine::GetRunSpeed() const
        {
            return m_playerParameter.runSpeed * m_parameter.runSpeed;
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


        IState* ZombieStateMachine::GetChangeState()
        {
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
