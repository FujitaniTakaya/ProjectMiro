/**
 * @file ZombieStateMachine.cpp
 * @brief ゾンビのステートマシン
 */
#include "stdafx.h"

#include "ZombieStateMachine.h"

#include "Source/Actor/Character/CharacterIState.h"
#include "Source/Actor/Character/Miro/MiroParameter.h"
#include "Source/Core/Param/ParamHolder.h"
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
            // 共通ステートを登録する
            AddState<CharacterIdleState>(this);
            AddState<CharacterWalkState>(this);
            AddState<CharacterRunState>(this);
        }


        float ZombieStateMachine::GetWalkSpeed() const
        {
            return m_playerParameter.runSpeed * m_parameter.walkSpeed;
        }


        float ZombieStateMachine::GetRunSpeed() const
        {
            return m_playerParameter.runSpeed * m_parameter.runSpeed;
        }


        IState* ZombieStateMachine::GetChangeState()
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
