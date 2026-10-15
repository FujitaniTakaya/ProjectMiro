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




        /*************************************************************/


        void CharacterWalkState::Enter()
        {
            m_owner->PlayAnimation(EnCharacterAnimationClipID::Walk);
        }


        void CharacterWalkState::Update()
        {
            m_owner->SetMoveSpeed(m_owner->GetWalkSpeed());
            m_owner->Move();
        }


        void CharacterWalkState::Exit()
        {
        }


        CharacterWalkState::CharacterWalkState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }




        /*************************************************************/


        void CharacterRunState::Enter()
        {
            m_owner->PlayAnimation(EnCharacterAnimationClipID::Run);
        }


        void CharacterRunState::Update()
        {
            m_owner->SetMoveSpeed(m_owner->GetRunSpeed());
            m_owner->Move();
        }


        void CharacterRunState::Exit()
        {
        }


        CharacterRunState::CharacterRunState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }
    } // namespace actor
} // namespace app
