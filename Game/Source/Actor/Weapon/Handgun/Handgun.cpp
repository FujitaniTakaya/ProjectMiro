/**
 * @file Handgun.cpp
 * @brief ハンドガン
 */
#include "stdafx.h"

#include "Handgun.h"

#include <cstddef>
#include <cstdint>
#include <iterator>

#include "Source/Actor/ModelData.h"
#include "Source/Actor/Weapon/WeaponParameter.h"
#include "Source/Core/Param/ParamHolder.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャ(Glock17_BaseColor.DDS・Glock17_NormalMap.DDS)は同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/Weapon/Glock17/Glock17.tkm";
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = true;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = true;

            /** アニメーションクリップの番号。値はそのまま、クリップの並びの番号になる */
            enum class EnHandgunAnimationClipID : uint8_t
            {
                /** 撃つ(スライドが動く) */
                Fire,
                /** リロード(マガジンが動く) */
                Reload,

                Max
            };

            /**
             * アニメーションデータ
             * EnHandgunAnimationClipID の順に並べること。
             */
            constexpr AnimationData ANIMATION_DATA[] = {
                { "Assets/animData/Weapon/Glock17/Glock17_Fire.tka", false },
                { "Assets/animData/Weapon/Glock17/Glock17_Reload.tka", false },
            };
            static_assert(std::size(ANIMATION_DATA) == static_cast<size_t>(EnHandgunAnimationClipID::Max), "アニメーションデータの数をIDと合わせること");

            /** モデルデータ(Z-upのモデル) */
            constexpr ModelData MODEL_DATA = {
                MODEL_PATH,
                ANIMATION_DATA,
                EnModelUpAxis::enModelUpAxisZ,
                static_cast<uint8_t>(std::size(ANIMATION_DATA))
            };
        } // namespace


        Handgun::Handgun()
            : m_isReloading(false)
            , m_reloadTimer(0.0f)
            , m_animationClips(nullptr)
        {}


        Handgun::~Handgun() = default;


        bool Handgun::Fire()
        {
            if (m_isReloading || m_remainingAmmo == 0)
            {
                return false;
            }
            --m_remainingAmmo;
            m_modelRender.PlayAnimation(static_cast<uint8_t>(EnHandgunAnimationClipID::Fire), 0.0f, true);
            return true;
        }


        void Handgun::Reload()
        {
            if (m_isReloading || m_remainingAmmo >= m_magazineSize)
            {
                return;
            }

            if (m_reloadTime <= 0.0f)
            {
                m_remainingAmmo = m_magazineSize;
                return;
            }
            m_isReloading = true;
            m_reloadTimer = m_reloadTime;
            m_modelRender.PlayAnimation(static_cast<uint8_t>(EnHandgunAnimationClipID::Reload), 0.0f, true);
        }


        void Handgun::Start()
        {
            // NOTE: パラメーターはコピーするので、jsonのホットリロードは次にStart()を呼ぶまで反映されない。
            Init(ParamHolder::Get().GetParameter<WeaponParameter>(EnParamID::Handgun));
            m_isReloading = false;
            m_reloadTimer = 0.0f;

            // アニメーションクリップを同期ロード
            m_animationClips = LoadAnimationClips(MODEL_DATA);

            // モデルのロード。先頭のクリップ(撃つ)が、ロード直後に1回再生される
            m_modelRender.Init(MODEL_DATA.fileName, m_animationClips.get(), MODEL_DATA.clipNum, MODEL_DATA.upAxis, IS_RECEIVE_SHADOW, IS_CAST_SHADOW);

            // 初回の描画前にワールド行列を確定させる
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void Handgun::Update()
        {
            if (m_isReloading)
            {
                m_reloadTimer -= g_gameTime->GetFrameDeltaTime();
                if (m_reloadTimer <= 0.0f)
                {
                    m_isReloading = false;
                    m_reloadTimer = 0.0f;
                    m_remainingAmmo = m_magazineSize;
                }
            }

            // モデルの更新
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void Handgun::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }
    } // namespace actor
} // namespace app
