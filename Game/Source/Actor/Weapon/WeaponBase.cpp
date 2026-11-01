/**
 * @file WeaponBase.cpp
 * @brief ウェポンの基底クラス
 */
#include "stdafx.h"

#include "WeaponBase.h"

#include "WeaponParameter.h"


namespace app
{
    namespace actor
    {
        WeaponBase::WeaponBase()
            : m_reloadTime(0.0f)
            , m_magazineSize(0)
            , m_remainingAmmo(0)
            , m_parameter(nullptr)
            , m_attachBone(nullptr)
        {
        }


        void WeaponBase::Init(const WeaponParameter& param)
        {
            m_reloadTime = param.reloadTime;
            m_magazineSize = param.magazineSize;
            m_remainingAmmo = m_magazineSize;
            m_parameter = &param;
        }


        void WeaponBase::AttachToBone(const Bone* bone)
        {
            m_attachBone = bone;
        }


        void WeaponBase::Detach()
        {
            m_attachBone = nullptr;
        }


        void WeaponBase::UpdateAttachedTransform()
        {
            if (m_attachBone == nullptr)
            {
                return;
            }

            // 握り(武器の空間 → 手のボーンの空間)。パラメーターが無い間は、手のボーンの位置と向きのまま
            Matrix grip;
            if (m_parameter != nullptr)
            {
                Matrix gripRotation;
                gripRotation.MakeRotationFromQuaternion(m_parameter->gripRotation);
                Matrix gripTranslation;
                gripTranslation.MakeTranslation(m_parameter->gripPosition);
                grip = gripRotation * gripTranslation;
            }

            // ModelRenderは、トランスフォームに「モデルの上方向の補正」を先に掛けて、ワールド行列にする。(補正 * 拡大 * 回転 * 平行移動)
            // 武器のワールド行列を (握り * ボーンのワールド行列) に合わせるため、補正を打ち消した行列を、トランスフォームにする。
            const Matrix upAxisBias = m_modelRender.GetModel().CalcWorldMatrix(Vector3::Zero, Quaternion::Identity, Vector3::One);
            Matrix upAxisBiasInverse;
            upAxisBiasInverse.Inverse(upAxisBias);
            const Matrix world = upAxisBiasInverse * grip * m_attachBone->GetWorldMatrix();

            // 行列(拡大 * 回転 * 平行移動)を、座標・回転・拡大に分ける
            Matrix rotation;
            Vector3 scale;
            for (int row = 0; row < 3; ++row)
            {
                Vector3 axis(world.m[row][0], world.m[row][1], world.m[row][2]);
                scale.v[row] = axis.Length();
                axis.Normalize();
                rotation.m[row][0] = axis.x;
                rotation.m[row][1] = axis.y;
                rotation.m[row][2] = axis.z;
            }
            m_transform.m_position.Set(world.m[3][0], world.m[3][1], world.m[3][2]);
            m_transform.m_rotation.SetRotation(rotation);
            m_transform.m_scale = scale;
        }
    } // namespace actor
} // namespace app
