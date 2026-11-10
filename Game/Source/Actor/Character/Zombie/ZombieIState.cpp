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
        void ZombieIdleState::Enter()
        {
            // 共通の待機(移動速度0・待機のアニメーション)に入ってから、慣性を消してその場で止める
            CharacterIdleState::Enter();
            m_owner->StopMoving();
        }


        ZombieIdleState::ZombieIdleState(CharacterStateMachine* owner)
            : CharacterIdleState(owner)
        {
        }




        /*************************************************************/


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




        /*************************************************************/


        void ZombieAttackState::Enter()
        {
            // 待機と同じく、コアに着いた勢いで滑らないよう、慣性を消してその場で止める
            m_owner->SetMoveSpeed(0.0f);
            m_owner->StopMoving();
            m_owner->PlayAnimation(EnZombieAnimationClipID::Attack);
        }


        void ZombieAttackState::Update()
        {
            // 攻撃の動きが終わっても攻撃ボタンを押し続けていれば、最初から再生し直す(巻き戻すときはブレンドしない)
            if (!m_owner->IsPlayingAnimation() && m_owner->GetInput().IsPress(EnInputButton::ActionA))
            {
                m_owner->PlayAnimation(EnZombieAnimationClipID::Attack, 0.0f, true);
            }

            // 入力がないので止まる(慣性で減速して、重力を受ける)
            m_owner->Move();
        }


        void ZombieAttackState::Exit()
        {
        }


        ZombieAttackState::ZombieAttackState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }
    } // namespace actor
} // namespace app
