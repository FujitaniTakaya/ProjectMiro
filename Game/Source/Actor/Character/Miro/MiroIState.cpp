/**
 * @file MiroIState.cpp
 * @brief Miro専用のステート
 */
#include "stdafx.h"

#include "MiroIState.h"

#include "MiroStateMachine.h"
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {


        void MiroMoveState::Enter()
        {
            PlayDirectionalAnimation();
        }


        void MiroMoveState::Update()
        {
            PlayDirectionalAnimation();
            m_owner->SetMoveSpeed(GetMoveSpeed());
            m_owner->Move();
        }


        void MiroMoveState::Exit()
        {
        }


        void MiroMoveState::PlayDirectionalAnimation()
        {
            m_owner->PlayAnimation(GetClip(m_miroOwner->CalcMoveDirection()));
        }


        MiroMoveState::MiroMoveState(MiroStateMachine* owner)
            : CharacterIState(owner)
            , m_miroOwner(owner)
        {
        }




        /*************************************************************/


        float MiroWalkState::GetMoveSpeed() const
        {
            return m_owner->GetWalkSpeed();
        }


        EnMiroAnimationClipID MiroWalkState::GetClip(const EnMiroMoveDirection direction) const
        {
            return GetMiroWalkClip(direction);
        }


        MiroWalkState::MiroWalkState(MiroStateMachine* owner)
            : MiroMoveState(owner)
        {
        }




        /*************************************************************/


        float MiroRunState::GetMoveSpeed() const
        {
            return m_owner->GetRunSpeed();
        }


        EnMiroAnimationClipID MiroRunState::GetClip(const EnMiroMoveDirection direction) const
        {
            return GetMiroRunClip(direction);
        }


        MiroRunState::MiroRunState(MiroStateMachine* owner)
            : MiroMoveState(owner)
        {
        }




        /*************************************************************/


        void MiroAimState::Enter()
        {
            m_isFiring = false;

            // 上半身=構える動き、下半身=歩き/待機
            m_owner->PlayLayerAnimation(EnAnimationLayer::Upper, EnMiroAnimationClipID::GunShot01, m_miroOwner->GetUpperBodyBlendTime());
            m_miroOwner->PlayLowerBodyAnimation();
        }


        void MiroAimState::Update()
        {
            // 歩きより遅い構え中の速度で動く。毎フレーム読むので、ホットリロードが効く
            m_owner->SetMoveSpeed(m_miroOwner->GetAimMoveSpeed());
            m_miroOwner->PlayLowerBodyAnimation();

            // 上半身の動きが終わって構えたポーズを保っている間(構える動きと撃つ動きが終わった後)に、ZR(ActionRB2)を押した瞬間に1発撃つ。
            // 撃てたら、上半身の撃つ動き(GunShot02)を最初から再生する。始まりと終わりが構えたポーズなので、終わるとそのポーズを保つ。下半身は歩き続ける
            if (!m_owner->IsPlayingLayerAnimation(EnAnimationLayer::Upper) && m_owner->GetInput().IsTrigger(EnInputButton::ActionRB2) && m_miroOwner->TryFire())
            {
                m_owner->PlayLayerAnimation(EnAnimationLayer::Upper, EnMiroAnimationClipID::GunShot02, 0.0f, true);
                m_isFiring = true;
            }

            m_owner->Move();
        }


        void MiroAimState::Exit()
        {
        }


        bool MiroAimState::IsFiring() const
        {
            // 撃つ動きが終われば、撃つ動きの最中ではなくなる。(フラグは、構える動きの間は立たない)
            return m_isFiring && m_owner->IsPlayingLayerAnimation(EnAnimationLayer::Upper);
        }


        MiroAimState::MiroAimState(MiroStateMachine* owner)
            : CharacterIState(owner)
            , m_miroOwner(owner)
            , m_isFiring(false)
        {
        }




        /*************************************************************/


        void MiroAimEndState::Enter()
        {
            // 上半身=構えを解く動き、下半身=歩き/待機
            m_owner->PlayLayerAnimation(EnAnimationLayer::Upper, EnMiroAnimationClipID::GunShot03, m_miroOwner->GetUpperBodyBlendTime());
            m_miroOwner->PlayLowerBodyAnimation();
        }


        void MiroAimEndState::Update()
        {
            // 立ち止まらず、構え中と同じ速度で動き続ける。毎フレーム読むので、ホットリロードが効く
            m_owner->SetMoveSpeed(m_miroOwner->GetAimMoveSpeed());
            m_miroOwner->PlayLowerBodyAnimation();
            m_owner->Move();
        }


        void MiroAimEndState::Exit()
        {
        }


        MiroAimEndState::MiroAimEndState(MiroStateMachine* owner)
            : CharacterIState(owner)
            , m_miroOwner(owner)
        {
        }




        /*************************************************************/


        void MiroReloadState::Enter()
        {
            // 上半身=リロードの動き、下半身=歩き/待機
            m_owner->PlayLayerAnimation(EnAnimationLayer::Upper, EnMiroAnimationClipID::PistolReload, m_miroOwner->GetUpperBodyBlendTime());
            m_miroOwner->PlayLowerBodyAnimation();
            m_miroOwner->StartReload();
        }


        void MiroReloadState::Update()
        {
            // 構え中と同じ、歩きより遅い速度で動く。毎フレーム読むので、ホットリロードが効く
            m_owner->SetMoveSpeed(m_miroOwner->GetAimMoveSpeed());
            m_miroOwner->PlayLowerBodyAnimation();
            m_owner->Move();
        }


        void MiroReloadState::Exit()
        {
        }


        MiroReloadState::MiroReloadState(MiroStateMachine* owner)
            : CharacterIState(owner)
            , m_miroOwner(owner)
        {
        }
    } // namespace actor
} // namespace app
