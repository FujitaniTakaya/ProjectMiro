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
            // 待機(慣性を消すゾンビ用)・歩き・走りを登録する
            AddState<ZombieIdleState>(this);
            AddState<ZombieWalkState>(this);
            AddState<ZombieRunState>(this);
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
                return FindState(ZombieRunState::ID());
            }

            // 2. 移動入力があるなら歩きへ
            if (CanChangeMoveState())
            {
                return FindState(ZombieWalkState::ID());
            }

            // どれにも当てはまらなければ待機へ
            return FindState(ZombieIdleState::ID());
        }
    } // namespace actor
} // namespace app
