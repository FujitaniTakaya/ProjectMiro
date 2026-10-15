/**
 * @file MiroStateMachine.cpp
 * @brief Miroのステートマシン
 */
#include "stdafx.h"

#include "MiroStateMachine.h"

#include "MiroParameter.h"
#include "Source/Actor/Character/CharacterIState.h"


namespace app
{
    namespace actor
    {
        MiroStateMachine::MiroStateMachine(CharacterBase* ownerCharacter, const MiroParameter& parameter)
            : CharacterStateMachine(ownerCharacter)
            , m_parameter(parameter)
        {
            // 共通ステートを登録する
            AddState<CharacterIdleState>(this);
            AddState<CharacterWalkState>(this);
            AddState<CharacterRunState>(this);
        }


        float MiroStateMachine::GetWalkSpeed() const
        {
            return m_parameter.walkSpeed;
        }


        float MiroStateMachine::GetRunSpeed() const
        {
            return m_parameter.runSpeed;
        }


        IState* MiroStateMachine::GetChangeState()
        {
            // 1. 走りに切り替えられるなら走りへ
            if (CanChangeRunState())
            {
                return FindState(CharacterRunState::ID());
            }

            // 2. 移動入力があるなら歩きへ
            if (CanChangeMoveState())
            {
                return FindState(CharacterWalkState::ID());
            }

            // どれにも当てはまらなければ待機へ
            return FindState(CharacterIdleState::ID());
        }
    } // namespace actor
} // namespace app
