/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 */
#include "stdafx.h"

#include "CharacterBase.h"

#include <algorithm>


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = true;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = true;
            /** SnapToGround() の1回の落とす距離(cm)。1フレームの移動の大きさが15cmを超えると、デバッグ用のログが出るので、それ以下にすること */
            constexpr float SNAP_STEP_DISTANCE = 14.0f;
            /** SnapToGround() で落とす回数の上限。1回の距離との積(5600cm)が、落とせる高さの上限になる */
            constexpr int SNAP_MAX_STEP_COUNT = 400;
        } // namespace


        CharacterBase::CharacterBase()
            : m_animationClips(nullptr)
            , m_currentVelocity(Vector3::Zero)
            , m_verticalVelocity(0.0f)
            , m_hp(0.0f)
        {}


        CharacterBase::~CharacterBase() = default;


        void CharacterBase::Update()
        {
            // モデルの行列を更新
            // アニメーション進行とボーン行列の計算を含む
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void CharacterBase::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }


        void CharacterBase::SetHp(const float hp)
        {
            // NOTE: 最大HPが0以下(パラメーターの誤り)でも、std::clampの前提(下限 <= 上限)が崩れないようにする。
            const float maxHp = GetMaxHp();
            m_hp = (maxHp > 0.0f) ? std::clamp(hp, 0.0f, maxHp) : 0.0f;
        }


        void CharacterBase::TakeDamage(const float damage)
        {
            if (damage <= 0.0f)
            {
                return;
            }
            SetHp(m_hp - damage);
        }


        void CharacterBase::Heal(const float amount)
        {
            if (amount <= 0.0f)
            {
                return;
            }
            SetHp(m_hp + amount);
        }


        void CharacterBase::SnapToGround()
        {
            const Vector3 originalPosition = m_transform.m_position;

            // 1回に SNAP_STEP_DISTANCE だけ落とす。時間は1秒として渡すので、速さ = 距離
            Vector3 position = originalPosition;
            for (int i = 0; i < SNAP_MAX_STEP_COUNT; ++i)
            {
                Vector3 moveSpeed(0.0f, -SNAP_STEP_DISTANCE, 0.0f);
                m_characterController.SetPosition(position);
                position = m_characterController.Execute(moveSpeed, 1.0f);
                if (m_characterController.IsOnGround())
                {
                    // 着いた。モデルも同じ位置へ置く
                    m_transform.m_position = position;
                    m_modelRender.SetTRS(m_transform);
                    m_modelRender.Update();
                    return;
                }
            }

            // 下に地面が無かった。座標は元に戻す。(物理の剛体は、次に Execute() した時に追従する)
            m_characterController.SetPosition(originalPosition);
        }


        void CharacterBase::Init(const ModelData& data, const float radius, const float height)
        {
            // アニメーションクリップを同期ロード
            m_animationClips = LoadAnimationClips(data);

            // モデルを初期化
            m_modelRender.Init(
                data.fileName,
                m_animationClips.get(),
                data.clipNum,
                data.upAxis,
                IS_RECEIVE_SHADOW,
                IS_CAST_SHADOW
            );

            // 初回の描画前にワールド行列を確定させる
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();

            // キャラクターコントローラーを初期化
            m_characterController.Init(radius, height, m_transform.m_position);

            // 現在のHPは、最大HPで始める
            m_hp = GetMaxHp();
        }
    } // namespace actor
} // namespace app
