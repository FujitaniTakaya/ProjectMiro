/**
 * @file MiroStateMachine.cpp
 * @brief Miroのステートマシン
 */
#include "stdafx.h"

#include "MiroStateMachine.h"

#include <cmath>

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
            // 待機は共通ステート、歩き・走り・構えはMiro専用のステートを登録する
            AddState<CharacterIdleState>(this);
            AddState<MiroWalkState>(this);
            AddState<MiroRunState>(this);
            AddState<MiroAimState>(this);
            AddState<MiroAimEndState>(this);
        }


        float MiroStateMachine::GetWalkSpeed() const
        {
            return m_parameter.walkSpeed;
        }


        float MiroStateMachine::GetRunSpeed() const
        {
            return m_parameter.runSpeed;
        }


        float MiroStateMachine::GetAimMoveSpeed() const
        {
            return m_parameter.aimMoveSpeed;
        }


        void MiroStateMachine::UpdateRotation()
        {
            // カメラの方向が指定されていれば、移動方向と関係なく、その方向を向く(入力がなくても向く)
            const CharacterInput& input = GetInput();
            if (input.HasLook())
            {
                Vector3 facing = input.GetLook();

                // 構え中は、画面の中央側(右)へ少し回す。右は、PlayerController の cameraRight と同じ向き
                if (IsAiming())
                {
                    const float turnRadian = Math::DegToRad(m_parameter.aimTurnDegrees);
                    const Vector3 look = input.GetLook();
                    const Vector3 right(look.z, 0.0f, -look.x);
                    facing = look * cosf(turnRadian) + right * sinf(turnRadian);
                }

                FaceDirection(facing);
                return;
            }

            // 指定がなければ、共通の動き(移動方向を向く)
            CharacterStateMachine::UpdateRotation();
        }


        bool MiroStateMachine::CanChangeAimEndState() const
        {
            // 構えていた直後(ZLを離した)か、構えを解く動きの途中
            return IsEqualCurrentState(MiroAimState::ID()) || (IsEqualCurrentState(MiroAimEndState::ID()) && IsPlayingAnimation());
        }


        bool MiroStateMachine::IsAiming() const
        {
            return IsEqualCurrentState(MiroAimState::ID()) || IsEqualCurrentState(MiroAimEndState::ID());
        }


        IState* MiroStateMachine::GetChangeState()
        {
            // 1. ZLを押している間は構えへ(走っていても、歩いていても構える)
            if (CanChangeAimState())
            {
                return FindState(MiroAimState::ID());
            }

            // 2. ZLを離した直後は、構えを解く動きへ(動きが終わるまで)
            if (CanChangeAimEndState())
            {
                return FindState(MiroAimEndState::ID());
            }

            // 3. 走りに切り替えられるなら走りへ
            if (CanChangeRunState())
            {
                return FindState(MiroRunState::ID());
            }

            // 4. 移動入力があるなら歩きへ
            if (CanChangeMoveState())
            {
                return FindState(MiroWalkState::ID());
            }

            // どれにも当てはまらなければ待機へ
            return FindState(CharacterIdleState::ID());
        }
    } // namespace actor
} // namespace app
