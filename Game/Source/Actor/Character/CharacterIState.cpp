/**
 * @file CharacterIState.cpp
 * @brief キャラクター共通のステート
 */
#include "stdafx.h"

#include "CharacterIState.h"

#include "CharacterAnimationClipID.h"
#include "CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        CharacterIState::CharacterIState(CharacterStateMachine* owner)
            : m_owner(owner)
        {
        }




        /*************************************************************/


        void CharacterIdleState::Enter()
        {
            m_owner->SetMoveSpeed(0.0f);
            m_owner->PlayAnimation(EnCharacterAnimationClipID::Idle);
        }


        void CharacterIdleState::Update()
        {
            m_owner->Move();
        }


        void CharacterIdleState::Exit()
        {
        }


        CharacterIdleState::CharacterIdleState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }
    } // namespace actor
} // namespace app
