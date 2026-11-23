/**
 * @file Bullet.cpp
 * @brief 弾
 */
#include "stdafx.h"

#include "Bullet.h"

#include "BulletParameter.h"
#include "BulletSweep.h"
#include "Source/Core/Param/ParamHolder.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャ(T_PropsGun_BaseColor.DDS)は同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/Weapon/Bullet/Glock17_Bullet.tkm";
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = false;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = false;
        } // namespace


        Bullet::Bullet()
            : m_parameter(ParamHolder::Get().GetParameter<BulletParameter>(EnParamID::Bullet))
            , m_speed(0.0f)
            , m_elapsedTime(0.0f)
            , m_ignore(nullptr)
            , m_isHit(false)
            , m_hit()
        {}


        void Bullet::Init(const Quaternion& rotation, const Vector3& position, const float speed)
        {
            m_transform.m_rotation = rotation;
            m_transform.m_position = position;
            m_speed = speed;
            m_elapsedTime = 0.0f;
            m_isHit = false;
        }


        bool Bullet::IsExpired() const
        {
            return m_elapsedTime >= m_parameter.lifeTime;
        }


        void Bullet::Start()
        {
            const float scale = m_parameter.modelScale;
            m_transform.m_scale = Vector3(scale, scale, scale);

            // アニメーションは無い。Z-upのモデル(Handgunと同じ出力手順)
            m_modelRender.Init(MODEL_PATH, nullptr, 0, EnModelUpAxis::enModelUpAxisZ, IS_RECEIVE_SHADOW, IS_CAST_SHADOW);

            // 初回の描画前にワールド行列を確定させる
            m_modelRender.SetTRS(MakeRenderTransform());
            m_modelRender.Update();
        }


        void Bullet::Update()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();
            m_elapsedTime += deltaTime;

            // 発射角度の向きへ進む。当たった弾は、止まったまま(次に破棄される)
            if (!m_isHit)
            {
                Vector3 forward = Vector3::Front;
                m_transform.m_rotation.Apply(forward);
                const Vector3 previous = m_transform.m_position;
                m_transform.m_position += forward * (m_speed * deltaTime);

                // 前の位置から今の位置まで球を動かして、何かに当たったら、当たった所で止める。
                // (1フレームで数十cm進むので、位置だけを調べると、ゾンビや薄い物をすり抜ける)
                if (SweepSphere(previous, m_transform.m_position, m_parameter.hitRadius, m_ignore, m_hit))
                {
                    m_transform.m_position = m_hit.position;
                    m_isHit = true;
                }
            }

            // 拡大率はホットリロードを反映させるため、毎フレーム読む
            const float scale = m_parameter.modelScale;
            m_transform.m_scale = Vector3(scale, scale, scale);

            m_modelRender.SetTRS(MakeRenderTransform());
            m_modelRender.Update();
        }


        void Bullet::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }


        Transform Bullet::MakeRenderTransform() const
        {
            // ParamLoader::ToRotation() と同じ順(Y → X → Z)で、度数のオイラー角を回転にする。
            // 先にモデルの補正を掛けてから、飛ぶ向きの回転を掛ける。(モデルの向きだけが変わり、進む向きは変わらない)
            Quaternion modelRotationX;
            modelRotationX.SetRotationDegX(m_parameter.modelRotation.x);
            Quaternion modelRotationY;
            modelRotationY.SetRotationDegY(m_parameter.modelRotation.y);
            Quaternion modelRotationZ;
            modelRotationZ.SetRotationDegZ(m_parameter.modelRotation.z);

            Quaternion rotation = modelRotationY;
            rotation *= modelRotationX;
            rotation *= modelRotationZ;
            rotation *= m_transform.m_rotation;

            Transform renderTransform = m_transform;
            renderTransform.m_rotation = rotation;
            return renderTransform;
        }
    } // namespace actor
} // namespace app
