/**
 * @file CharacterStateMachine.cpp
 * @brief キャラクターのステートマシン
 */
#include "stdafx.h"

#include "CharacterStateMachine.h"

#include <algorithm>


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 重力の値 */
            constexpr float GRAVITY = -9.8f * 150.0f;
            /** 加速度(目標速度への到達の早さ) */
            constexpr float ACCELERATION = 10.0f;
            /** 摩擦係数(入力がないときの減速の早さ) */
            constexpr float FRICTION = 5.0f;
            /** 補間の割合の最大値 */
            constexpr float MAX_LERP_FACTOR = 1.0f;
        } // namespace


        void CharacterStateMachine::Move()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();
            const Vector3& inputMove = m_input.GetMove();

            // --- 慣性と摩擦の計算 ---
            // 入力の長さは倒し具合(0.0f〜1.0f)なので、そのまま速度の割合になる
            Vector3 velocity = m_ownerCharacter->GetCurrentVelocity();
            if (m_input.IsMoving())
            {
                const Vector3 targetVelocity = inputMove * m_moveSpeed;
                const float lerpFactor = std::min<float>(MAX_LERP_FACTOR, ACCELERATION * deltaTime);
                velocity.Lerp(lerpFactor, velocity, targetVelocity);
            }
            else
            {
                const float lerpFactor = std::min<float>(MAX_LERP_FACTOR, FRICTION * deltaTime);
                velocity.Lerp(lerpFactor, velocity, Vector3::Zero);
            }
            m_ownerCharacter->SetCurrentVelocity(velocity);

            // --- キャラクターコントローラーによるコリジョン解決 ---
            // 持ち主の座標が正なので、外から SetPosition() されていてもその座標から動くよう、毎回渡す
            CharacterController* characterController = m_ownerCharacter->GetCharacterController();
            characterController->SetPosition(m_ownerActor->GetTransform().m_position);

            // 素のCharacterControllerには重力がないので、垂直方向の速度に重力を足して渡す
            Vector3 moveSpeed(velocity.x, m_ownerCharacter->GetVerticalVelocity() + GRAVITY * deltaTime, velocity.z);
            const Vector3 nextPosition = characterController->Execute(moveSpeed, deltaTime);
            // 着地すると、Execute() が垂直方向の速度を0に戻す
            m_ownerCharacter->SetVerticalVelocity(moveSpeed.y);
            m_ownerActor->SetPosition(nextPosition);

            // --- 回転 ---
            // 入力がある間だけ、入力の方向へ向ける。(入力がない間は、向きを変えない)
            if (m_input.IsMoving())
            {
                Vector3 inputDirection = inputMove;
                inputDirection.y = 0.0f;
                inputDirection.Normalize();

                Quaternion rotation = m_ownerActor->GetTransform().m_rotation;
                rotation.SetRotationYFromDirectionXZ(inputDirection);
                m_ownerActor->SetRotation(rotation);
            }
        }


        IState* CharacterStateMachine::GetChangeState()
        {
            // 何もしない
            return nullptr;
        }


        CharacterStateMachine::CharacterStateMachine(CharacterBase* ownerCharacter)
            : ActorStateMachine(ownerCharacter)
            , m_ownerCharacter(ownerCharacter)
            , m_input()
            , m_moveSpeed(0.0f)
        {
        }
    } // namespace actor
} // namespace app
