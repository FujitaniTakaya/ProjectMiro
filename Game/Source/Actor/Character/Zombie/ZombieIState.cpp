/**
 * @file ZombieIState.cpp
 * @brief ゾンビ専用のステート
 */
#include "stdafx.h"

#include "ZombieIState.h"

#include "Source/Actor/Character/CharacterStateMachine.h"
#include "ZombieStateMachine.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /**
             * @brief 下半身(Base)のクリップが切り替わったときだけ、個体ごとの開始位相から始める
             * @details 同じクリップを再生中に呼んでも、シークしない(動きが跳ばない)。
             * @param owner ゾンビのステートマシン
             * @param clipBefore 切り替える前に再生していたクリップの番号(GetPlayingClip() で取っておく)
             * @param clip 今再生しているクリップ
             */
            void ApplyPhaseOffset(CharacterStateMachine* owner, const int clipBefore, const EnZombieAnimationClipID clip)
            {
                if (clipBefore != static_cast<int>(clip))
                {
                    owner->SetLayerAnimationPhase(EnAnimationLayer::Base, static_cast<ZombieStateMachine*>(owner)->GetPhaseOffset());
                }
            }
        } // namespace


        void ZombieIdleState::Enter()
        {
            const int clipBefore = m_owner->GetPlayingClip(EnAnimationLayer::Base);

            // 共通の待機(移動速度0・待機のアニメーション)に入ってから、慣性を消してその場で止める
            CharacterIdleState::Enter();
            m_owner->StopMoving();
            ApplyPhaseOffset(m_owner, clipBefore, EnZombieAnimationClipID::Idle);
        }


        ZombieIdleState::ZombieIdleState(CharacterStateMachine* owner)
            : CharacterIdleState(owner)
        {
        }




        /*************************************************************/


        void ZombieMoveState::Enter()
        {
            m_owner->SetMoveSpeed(GetMoveSpeed());

            // 同じところから同時に湧いても足並みがそろわないよう、個体ごとの位相から始める
            const int clipBefore = m_owner->GetPlayingClip(EnAnimationLayer::Base);
            m_owner->PlayAnimation(GetClip());
            ApplyPhaseOffset(m_owner, clipBefore, GetClip());
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
            // 構えながら歩くプレイヤーに当たるよう、攻撃の動きを少し早送りする(抜けるときに戻す)
            m_owner->SetAnimationSpeed(static_cast<ZombieStateMachine*>(m_owner)->GetAttackAnimationSpeed());
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
            m_owner->SetAnimationSpeed(1.0f);
        }


        ZombieAttackState::ZombieAttackState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }




        /*************************************************************/


        void ZombieDeathState::Enter()
        {
            // 慣性で滑らないよう止めて、死亡アニメーションを再生する
            m_owner->SetMoveSpeed(0.0f);
            m_owner->StopMoving();
            m_owner->PlayAnimation(EnZombieAnimationClipID::Death);
        }


        void ZombieDeathState::Update()
        {
            // 何もしない。体は物理ワールドから外れているので、Move() も呼ばない
        }


        void ZombieDeathState::Exit()
        {
        }


        ZombieDeathState::ZombieDeathState(CharacterStateMachine* owner)
            : CharacterIState(owner)
        {
        }
    } // namespace actor
} // namespace app
