/**
 * @file MiroIState.cpp
 * @brief Miro専用のステート
 */
#include "stdafx.h"

#include "MiroIState.h"

#include <cstddef>
#include <iterator>

#include "MiroStateMachine.h"
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /**
             * 歩きのクリップ。EnMiroMoveDirection の順に並べること
             * @note 真横(Right/Left)は、WalkRight/WalkLeft ではなく斜め前のクリップを使う。
             *       WalkRight/WalkLeft は体が約60°横を向いたまま動く素材で、真横に歩くと後退のように見えるため。
             *       体を正面に向けた横歩きの素材ができたら、WalkRight/WalkLeft に戻す。
             */
            constexpr EnMiroAnimationClipID WALK_CLIPS[] = {
                EnMiroAnimationClipID::WalkFwd,
                EnMiroAnimationClipID::WalkFwdRight,
                EnMiroAnimationClipID::WalkFwdRight,
                EnMiroAnimationClipID::WalkBwdRight,
                EnMiroAnimationClipID::WalkBwd,
                EnMiroAnimationClipID::WalkBwdLeft,
                EnMiroAnimationClipID::WalkFwdLeft,
                EnMiroAnimationClipID::WalkFwdLeft,
            };
            static_assert(std::size(WALK_CLIPS) == static_cast<size_t>(EnMiroMoveDirection::Max), "歩きのクリップの数を方向と合わせること");

            /**
             * 走りのクリップ。EnMiroMoveDirection の順に並べること
             * @note 真横(Right/Left)は、歩きと同じ理由で、RunRight/RunLeft ではなく斜め前のクリップを使う。
             */
            constexpr EnMiroAnimationClipID RUN_CLIPS[] = {
                EnMiroAnimationClipID::RunFwd,
                EnMiroAnimationClipID::RunFwdRight,
                EnMiroAnimationClipID::RunFwdRight,
                EnMiroAnimationClipID::RunBwdRight,
                EnMiroAnimationClipID::RunBwd,
                EnMiroAnimationClipID::RunBwdLeft,
                EnMiroAnimationClipID::RunFwdLeft,
                EnMiroAnimationClipID::RunFwdLeft,
            };
            static_assert(std::size(RUN_CLIPS) == static_cast<size_t>(EnMiroMoveDirection::Max), "走りのクリップの数を方向と合わせること");
        } // namespace


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
            const CharacterInput& input = m_owner->GetInput();

            // 向いている方向。Lookがなければ、今の体の向き
            Vector3 look = Vector3::Front;
            if (input.HasLook())
            {
                look = input.GetLook();
            }
            else
            {
                m_owner->GetTransform().m_rotation.Apply(look);
            }

            const EnMiroMoveDirection direction = CalcMiroMoveDirection(look, input.GetMove());
            m_owner->PlayAnimation(GetClip(direction));
        }


        MiroMoveState::MiroMoveState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }




        /*************************************************************/


        float MiroWalkState::GetMoveSpeed() const
        {
            return m_owner->GetWalkSpeed();
        }


        EnMiroAnimationClipID MiroWalkState::GetClip(const EnMiroMoveDirection direction) const
        {
            return WALK_CLIPS[static_cast<size_t>(direction)];
        }


        MiroWalkState::MiroWalkState(CharacterStateMachine* owner)
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
            return RUN_CLIPS[static_cast<size_t>(direction)];
        }


        MiroRunState::MiroRunState(CharacterStateMachine* owner)
            : MiroMoveState(owner)
        {
        }




        /*************************************************************/


        void MiroAimState::Enter()
        {
            m_owner->PlayAnimation(EnMiroAnimationClipID::GunShot01);
        }


        void MiroAimState::Update()
        {
            // 歩きより遅い構え中の速度で動く。毎フレーム読むので、ホットリロードが効く
            m_owner->SetMoveSpeed(m_miroOwner->GetAimMoveSpeed());

            // 構えたポーズを保っている間(構える動きと撃つ動きが終わった後)に、ZRを押した瞬間に1発撃つ。
            // 撃てたら、撃つ動き(GunShot02)を最初から再生する。始まりと終わりが構えたポーズなので、終わるとそのポーズを保つ
            if (!m_owner->IsPlayingAnimation() && m_owner->GetInput().IsTrigger(EnInputButton::Fire) && m_miroOwner->TryFire())
            {
                m_owner->PlayAnimation(EnMiroAnimationClipID::GunShot02, 0.0f, true);
            }

            m_owner->Move();
        }


        void MiroAimState::Exit()
        {
        }


        MiroAimState::MiroAimState(MiroStateMachine* owner)
            : CharacterIState(owner)
            , m_miroOwner(owner)
        {
        }




        /*************************************************************/


        void MiroAimEndState::Enter()
        {
            m_owner->SetMoveSpeed(0.0f);
            m_owner->PlayAnimation(EnMiroAnimationClipID::GunShot03);
        }


        void MiroAimEndState::Update()
        {
            m_owner->Move();
        }


        void MiroAimEndState::Exit()
        {
        }


        MiroAimEndState::MiroAimEndState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }
    } // namespace actor
} // namespace app
