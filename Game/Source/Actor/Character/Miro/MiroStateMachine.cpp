/**
 * @file MiroStateMachine.cpp
 * @brief Miroのステートマシン
 */
#include "stdafx.h"

#include "MiroStateMachine.h"

#include <algorithm>
#include <cmath>
#include <utility>

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
            , m_weaponHandler()
        {
            // 待機は共通ステート、歩き・走り・構え・リロードはMiro専用のステートを登録する
            AddState<CharacterIdleState>(this);
            AddState<MiroWalkState>(this);
            AddState<MiroRunState>(this);
            AddState<MiroAimState>(this);
            AddState<MiroAimEndState>(this);
            AddState<MiroReloadState>(this);
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


        void MiroStateMachine::SetWeaponHandler(MiroWeaponHandler handler)
        {
            m_weaponHandler = std::move(handler);
        }


        bool MiroStateMachine::TryFire() const
        {
            return m_weaponHandler.fire && m_weaponHandler.fire();
        }


        bool MiroStateMachine::CanReload() const
        {
            return m_weaponHandler.canReload && m_weaponHandler.canReload();
        }


        bool MiroStateMachine::IsMagazineEmpty() const
        {
            return m_weaponHandler.isEmpty && m_weaponHandler.isEmpty();
        }


        void MiroStateMachine::StartReload() const
        {
            if (m_weaponHandler.reload)
            {
                m_weaponHandler.reload();
            }
        }


        void MiroStateMachine::PlayLowerBodyAnimation()
        {
            // 腰を構えたポーズの向きへ近づける割合。毎フレーム読むので、ホットリロードが効く(上半身が止まっている間は効かない)
            SetLayerFollowRatio(EnAnimationLayer::Upper, std::clamp(m_parameter.aimHipFollow, 0.0f, 1.0f));

            if (GetInput().IsMoving())
            {
                PlayAnimation(GetMiroWalkClip(CalcMoveDirection()));

                // 構え中の速度は歩きより遅い。足が地面を滑らないよう、速度の比だけ再生を遅くする。
                // (毎フレーム読むので、速度のホットリロードに追従する)
                const float ratio = (m_parameter.walkSpeed > 0.0f) ? (m_parameter.aimMoveSpeed / m_parameter.walkSpeed) : 1.0f;
                SetLayerAnimationSpeed(EnAnimationLayer::Base, ratio);
            }
            else
            {
                PlayAnimation(EnMiroAnimationClipID::Idle);
                SetLayerAnimationSpeed(EnAnimationLayer::Base, 1.0f);
            }
        }


        EnMiroMoveDirection MiroStateMachine::CalcMoveDirection() const
        {
            // 向いている方向。Lookがなければ、今の体の向き
            const CharacterInput& input = GetInput();
            Vector3 look = Vector3::Front;
            if (input.HasLook())
            {
                look = input.GetLook();
            }
            else
            {
                GetTransform().m_rotation.Apply(look);
            }
            return CalcMiroMoveDirection(look, input.GetMove());
        }


        float MiroStateMachine::GetUpperBodyBlendTime() const
        {
            return m_parameter.upperBodyBlendTime;
        }


        void MiroStateMachine::Update()
        {
            CharacterStateMachine::Update();

            // 銃を扱っていなければ、上半身を止める。(止まっているレイヤーに呼んでも、何も起きない)
            // 待機・歩き・走りのステートは、Base レイヤーの全身アニメーションだけを再生する。
            // 下半身の再生速度も、構え中に遅くしたのを戻す
            if (!IsHandlingGun())
            {
                StopLayerAnimation(EnAnimationLayer::Upper, m_parameter.upperBodyBlendTime);
                SetLayerAnimationSpeed(EnAnimationLayer::Base, 1.0f);
            }
        }


        void MiroStateMachine::UpdateRotation()
        {
            // カメラの方向が指定されていれば、移動方向と関係なく、その方向を向く(入力がなくても向く)
            const CharacterInput& input = GetInput();
            if (input.HasLook())
            {
                Vector3 facing = input.GetLook();

                // 構え中とリロード中は、画面の中央側(右)へ少し回す。右は、PlayerController の cameraRight と同じ向き
                if (IsHandlingGun())
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
            // 構えていた直後(ZLを離した)か、構えを解く動きの途中。
            // 動きは上半身のクリップ。(Base は歩き・待機のループなので、終わりが分からない)
            return IsEqualCurrentState(MiroAimState::ID()) || (IsEqualCurrentState(MiroAimEndState::ID()) && IsPlayingLayerAnimation(EnAnimationLayer::Upper));
        }


        bool MiroStateMachine::CanChangeReloadState()
        {
            // リロードのアニメーション(上半身のクリップ)が終わるまでは続ける
            if (IsEqualCurrentState(MiroReloadState::ID()) && IsPlayingLayerAnimation(EnAnimationLayer::Upper))
            {
                return true;
            }

            // 撃つ動きの最中は入らない。終わるまで待つ(最後の1発なら、終わった後で自動で入る)
            if (IsFiring() || !CanReload())
            {
                return false;
            }

            // X(ActionX)を押した瞬間と、マガジンが空の間(自動)
            return GetInput().IsTrigger(EnInputButton::ActionX) || IsMagazineEmpty();
        }


        bool MiroStateMachine::IsAiming() const
        {
            return IsEqualCurrentState(MiroAimState::ID()) || IsEqualCurrentState(MiroAimEndState::ID());
        }


        bool MiroStateMachine::IsFiring()
        {
            if (!IsEqualCurrentState(MiroAimState::ID()))
            {
                return false;
            }
            const MiroAimState* aimState = static_cast<const MiroAimState*>(FindState(MiroAimState::ID()));
            return aimState->IsFiring();
        }


        bool MiroStateMachine::IsHandlingGun() const
        {
            return IsAiming() || IsEqualCurrentState(MiroReloadState::ID());
        }


        IState* MiroStateMachine::GetChangeState()
        {
            // 1. リロードに入る(続ける)なら、リロードへ(構え中でも走っていても、リロードが優先)
            if (CanChangeReloadState())
            {
                return FindState(MiroReloadState::ID());
            }

            // 2. ZLを押している間は構えへ(走っていても、歩いていても構える)
            if (CanChangeAimState())
            {
                return FindState(MiroAimState::ID());
            }

            // 3. ZLを離した直後は、構えを解く動きへ(動きが終わるまで)
            if (CanChangeAimEndState())
            {
                return FindState(MiroAimEndState::ID());
            }

            // 4. 走りに切り替えられるなら走りへ
            if (CanChangeRunState())
            {
                return FindState(MiroRunState::ID());
            }

            // 5. 移動入力があるなら歩きへ
            if (CanChangeMoveState())
            {
                return FindState(MiroWalkState::ID());
            }

            // どれにも当てはまらなければ待機へ
            return FindState(CharacterIdleState::ID());
        }
    } // namespace actor
} // namespace app
