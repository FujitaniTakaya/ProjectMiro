/**
 * @file WeaponBase.cpp
 * @brief ウェポンの基底クラス
 */
#include "stdafx.h"

#include "WeaponBase.h"

#include <string>

#include "WeaponParameter.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /**
             * 行列(拡大 * 回転 * 平行移動)を、座標・回転・拡大に分ける
             * UpdateAttachedTransform() と CalcMuzzle() が使う。
             */
            void DecomposeWorld(const Matrix& world, Vector3& position, Quaternion& rotation, Vector3& scale)
            {
                Matrix rotationMatrix;
                for (int row = 0; row < 3; ++row)
                {
                    Vector3 axis(world.m[row][0], world.m[row][1], world.m[row][2]);
                    scale.v[row] = axis.Length();
                    axis.Normalize();
                    rotationMatrix.m[row][0] = axis.x;
                    rotationMatrix.m[row][1] = axis.y;
                    rotationMatrix.m[row][2] = axis.z;
                }
                position.Set(world.m[3][0], world.m[3][1], world.m[3][2]);
                rotation.SetRotation(rotationMatrix);
            }
        } // namespace


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
            DecomposeWorld(world, m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
        }


        bool WeaponBase::CalcMuzzle(Vector3& position, Quaternion& rotation) const
        {
            if (m_parameter == nullptr || m_parameter->muzzleBoneName.empty())
            {
                return false;
            }

            // 骨名はASCII。FindBone() はワイド文字列を取る
            const std::wstring boneName(m_parameter->muzzleBoneName.begin(), m_parameter->muzzleBoneName.end());
            const Bone* bone = m_modelRender.FindBone(boneName.c_str());
            if (bone == nullptr)
            {
                K2_LOG("WeaponBase::CalcMuzzle: ボーンが見つからない。name=%s\n", m_parameter->muzzleBoneName.c_str());
                return false;
            }

            // 銃口(ボーンの空間)→ ワールド。ボーンのワールド行列は、モデルのワールド行列(上方向の補正込み)を含む
            Matrix muzzleRotation;
            muzzleRotation.MakeRotationFromQuaternion(m_parameter->muzzleRotation);
            Matrix muzzleTranslation;
            muzzleTranslation.MakeTranslation(m_parameter->muzzlePosition);
            const Matrix world = muzzleRotation * muzzleTranslation * bone->GetWorldMatrix();

            Vector3 scale;
            DecomposeWorld(world, position, rotation, scale);
            return true;
        }
    } // namespace actor
} // namespace app
