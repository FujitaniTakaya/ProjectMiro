/**
 * @file ZombieIState.cpp
 * @brief ゾンビ専用のステート
 */
#include "stdafx.h"

#include "ZombieIState.h"

#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        void ZombieMoveState::Enter()
        {
            m_owner->SetMoveSpeed(GetMoveSpeed());
            m_owner->PlayAnimation(GetClip());
        }


        void ZombieMoveState::Update()
        {
            // ホットリロードで速度が変わっても追従するよう、毎フレーム設定する
            m_owner->SetMoveSpeed(GetMoveSpeed());
            m_owner->Move();
        }


        void ZombieMoveState::Exit()
        {
        }


        ZombieMoveState::ZombieMoveState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }




        /*************************************************************/


        float ZombieWalkState::GetMoveSpeed() const
        {
            return m_owner->GetWalkSpeed();
        }


        EnZombieAnimationClipID ZombieWalkState::GetClip() const
        {
            return EnZombieAnimationClipID::Walk;
        }


        ZombieWalkState::ZombieWalkState(CharacterStateMachine* owner)
            : ZombieMoveState(owner)
        {
        }




        /*************************************************************/


        float ZombieRunState::GetMoveSpeed() const
        {
            return m_owner->GetRunSpeed();
        }


        EnZombieAnimationClipID ZombieRunState::GetClip() const
        {
            return EnZombieAnimationClipID::Run;
        }


        ZombieRunState::ZombieRunState(CharacterStateMachine* owner)
            : ZombieMoveState(owner)
        {
        }
    } // namespace actor
} // namespace app
