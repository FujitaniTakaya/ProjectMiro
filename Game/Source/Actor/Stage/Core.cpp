/**
 * @file Core.cpp
 * @brief ステージのコア
 */
#include "stdafx.h"

#include "Core.h"

#include <algorithm>
#include <cmath>

#include "CoreParameter.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Manager/InGameInfoHub.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャ(Gemme_BaseColor.DDS・Gemme_NormalMap.DDS・Gemme_SpecularMap.DDS)は同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/stage/core/Gemme.tkm";
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = true;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = true;
            /** 拡大率の初期値。モデルがメートル単位なので、cm単位のワールドに合わせて100倍にして、さらに1.6倍にする */
            constexpr float MODEL_SCALE = 100.0f * 1.6f;
            /** 回転速度(度/秒)。6秒で一周する */
            constexpr float ROTATE_SPEED_DEG = 60.0f;
            /** X軸まわりの傾き(度)。正の値で、上端が+Z側へ倒れる */
            constexpr float TILT_X_DEG = 15.0f;
            /** 浮かせる高さ(cm)。宝石の下端から地面までの距離 */
            constexpr float FLOAT_HEIGHT = 70.0f;
            /** モデル空間(Z-up・拡大前)でのバウンディングボックスの中心。モデルの原点はここから少しずれているので、回転の軸にはこちらを使う */
            constexpr float MODEL_CENTER_X = 0.0646f;
            constexpr float MODEL_CENTER_Y = 0.0600f;
            constexpr float MODEL_CENTER_Z = 0.0511f;
            /** モデル空間(拡大前)での高さの半分。バウンディングボックスの中心から下端までの距離 */
            constexpr float MODEL_HALF_HEIGHT = 0.6595f;
        } // namespace


        Core::Core()
            : m_parameter(ParamHolder::Get().GetParameter<CoreParameter>(EnParamID::Core))
            , m_yawDeg(0.0f)
            , m_hp(0.0f)
        {
            m_transform.m_scale = Vector3(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE);
        }


        Core::~Core() = default;


        float Core::GetMaxHp() const
        {
            return m_parameter.maxHp;
        }


        void Core::TakeDamage(const float damage)
        {
            if (damage <= 0.0f)
            {
                return;
            }
            SetHp(m_hp - damage);
        }


        void Core::Heal(const float amount)
        {
            if (amount <= 0.0f)
            {
                return;
            }
            SetHp(m_hp + amount);
        }


        Vector3 Core::GetHpBarWorldPosition() const
        {
            Vector3 position = CalcCenter();
            position.y += m_parameter.hpBarOffsetY;
            return position;
        }


        void Core::Start()
        {
            // アニメーションは無い。Z-upのモデル
            m_modelRender.Init(MODEL_PATH, nullptr, 0, EnModelUpAxis::enModelUpAxisZ, IS_RECEIVE_SHADOW, IS_CAST_SHADOW);

            // 初回の描画前にワールド行列を確定させる
            UpdateModel();

            // 現在の耐久値は、最大耐久値で始める。最初の耐久値を通知
            m_hp = GetMaxHp();
            InGameInfoHub::Get().NotifyCoreHp(m_hp, GetMaxHp());
        }


        void Core::Update()
        {
            m_yawDeg = std::fmod(m_yawDeg + ROTATE_SPEED_DEG * g_gameTime->GetFrameDeltaTime(), 360.0f);
            UpdateModel();

            // 耐久値の通知(値が変わった時だけ、UIに伝わる。ホットリロードで最大耐久値が変わった時も含む)
            InGameInfoHub::Get().NotifyCoreHp(m_hp, GetMaxHp());
        }


        void Core::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }


        void Core::UpdateModel()
        {
            // 先に宝石自身の軸(Y軸)で回してから、ワールドのX軸まわりに傾ける。傾きの向きは、回っても変わらない
            Quaternion spin;
            spin.SetRotationDegY(m_yawDeg);
            Quaternion tilt;
            tilt.SetRotationDegX(TILT_X_DEG);
            m_transform.m_rotation.Multiply(spin, tilt);

            // モデルの原点から宝石の中心へのベクトル。Z-upをY-upに直すと(x, y, z)は(x, z, -y)になり、拡大はその後にかかる
            const Vector3& scale = m_transform.m_scale;
            Vector3 centerOffset(MODEL_CENTER_X * scale.x, MODEL_CENTER_Z * scale.y, -MODEL_CENTER_Y * scale.z);
            m_transform.m_rotation.Apply(centerOffset);

            // 宝石の中心を、地面から浮かせた点に置く。原点は、回転した分だけずらして、中心が動かないようにする
            m_modelRender.SetTRS(CalcCenter() - centerOffset, m_transform.m_rotation, scale);
            m_modelRender.Update();
        }


        Vector3 Core::CalcCenter() const
        {
            Vector3 center = m_transform.m_position;
            center.y += MODEL_HALF_HEIGHT * m_transform.m_scale.y + FLOAT_HEIGHT;
            return center;
        }


        void Core::SetHp(const float hp)
        {
            // NOTE: 最大耐久値が0以下(パラメーターの誤り)でも、std::clampの前提(下限 <= 上限)が崩れないようにする。
            const float maxHp = GetMaxHp();
            m_hp = (maxHp > 0.0f) ? std::clamp(hp, 0.0f, maxHp) : 0.0f;
        }
    } // namespace actor
} // namespace app
