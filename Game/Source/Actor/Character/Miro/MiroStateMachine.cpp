/**
 * @file MiroStateMachine.cpp
 * @brief Miroのステートマシン
 */
#include "stdafx.h"

#include "MiroStateMachine.h"

#include "MiroIState.h"
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
            // 待機は共通ステート、歩き・走りはMiro専用(8方向)のステートを登録する
            AddState<CharacterIdleState>(this);
            AddState<MiroWalkState>(this);
            AddState<MiroRunState>(this);
        }


        float MiroStateMachine::GetWalkSpeed() const
        {
            return m_parameter.walkSpeed;
        }


        float MiroStateMachine::GetRunSpeed() const
        {
            return m_parameter.runSpeed;
        }


        void MiroStateMachine::UpdateRotation()
        {
            // カメラの方向が指定されていれば、移動方向と関係なく、その方向を向く(入力がなくても向く)
            const CharacterInput& input = GetInput();
            if (input.HasLook())
            {
                FaceDirection(input.GetLook());
                return;
            }

            // 指定がなければ、共通の動き(移動方向を向く)
            CharacterStateMachine::UpdateRotation();
        }


        IState* MiroStateMachine::GetChangeState()
        {
            // 1. 走りに切り替えられるなら走りへ
            if (CanChangeRunState())
            {
                return FindState(MiroRunState::ID());
            }

            // 2. 移動入力があるなら歩きへ
            if (CanChangeMoveState())
            {
                return FindState(MiroWalkState::ID());
            }

            // どれにも当てはまらなければ待機へ
            return FindState(CharacterIdleState::ID());
        }
    } // namespace actor
} // namespace app
