/**
 * @file CharacterIState.cpp
 * @brief キャラクター共通のステート
 */
#include "stdafx.h"

#include "CharacterIState.h"


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
        }


        void CharacterIdleState::Update()
        {
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
        }


        void CharacterWalkState::Update()
        {
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
        }


        void CharacterRunState::Update()
        {
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
