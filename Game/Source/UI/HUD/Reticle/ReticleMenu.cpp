/**
 * @file ReticleMenu.cpp
 * @brief レティクル(照準の十字)を動かすMenu
 */
#include "stdafx.h"

#include "ReticleMenu.h"

#include <algorithm>
#include <cstdint>

#include "Source/Core/Util/CRC32.h"
#include "Source/Core/Util/Curve.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 4本のバーのUIの名前(ハッシュ値)。上・下・左・右の順 */
            constexpr uint32_t BAR_KEYS[] = { Hash32("ReticleTop"), Hash32("ReticleBottom"), Hash32("ReticleLeft"), Hash32("ReticleRight") };
            /** 構え切った時の広がり具合。(1 = jsonの位置。小さいほど中央に集まる) */
            constexpr float AIM_FACTOR = 0.35f;
            /** 構えの切り替えにかかる時間(秒) */
            constexpr float AIM_BLEND_SECONDS = 0.12f;
            /** 1発撃つごとに足す広がり */
            constexpr float KICK_PER_SHOT = 0.8f;
            /** 連射しても、これ以上は足さない広がりの上限 */
            constexpr float KICK_MAX = 1.6f;
            /** 発砲の広がりが元に戻る速さ(広がり/秒) */
            constexpr float KICK_DECAY_PER_SECOND = 6.0f;
        } // namespace


        ReticleMenu::ReticleMenu()
            : m_bars{ nullptr, nullptr, nullptr, nullptr }
            , m_basePositions{ Vector3::Zero, Vector3::Zero, Vector3::Zero, Vector3::Zero }
            , m_aimRate(0.0f)
            , m_isAiming(false)
            , m_kick(0.0f)
        {}


        ReticleMenu::~ReticleMenu() = default;


        void ReticleMenu::InitializeLogic()
        {
            for (int i = 0; i < BAR_COUNT; ++i)
            {
                m_bars[i] = GetUI<UIIcon>(BAR_KEYS[i]);
                if (m_bars[i])
                {
                    m_basePositions[i] = m_bars[i]->m_transform.m_localTransform.m_position;
                }
                else
                {
                    K2_LOG("ReticleMenu: UIIcon %d番目のバー がありません。\n", i);
                }
            }

            ApplySpread();
        }


        void ReticleMenu::Update()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();

            // 構えの度合いを、構え中は1へ、そうでなければ0へ動かす
            const float aimStep = deltaTime / AIM_BLEND_SECONDS;
            m_aimRate = m_isAiming ? (std::min)(m_aimRate + aimStep, 1.0f) : (std::max)(m_aimRate - aimStep, 0.0f);
            // 発砲の広がりを、0へ戻す
            m_kick = (std::max)(m_kick - KICK_DECAY_PER_SECOND * deltaTime, 0.0f);

            ApplySpread();
            MenuBase::Update();
        }


        void ReticleMenu::SetAiming(const bool isAiming)
        {
            m_isAiming = isAiming;
        }


        void ReticleMenu::OnFired()
        {
            m_kick = (std::min)(m_kick + KICK_PER_SHOT, KICK_MAX);
        }


        void ReticleMenu::ApplySpread()
        {
            // 構えると、jsonの位置からAIM_FACTORの位置まで集まる。発砲の広がりは、その上に足す
            const float aimEased = util::ApplyEasing(util::EasingType::EaseOut, m_aimRate);
            const float factor = 1.0f + (AIM_FACTOR - 1.0f) * aimEased + m_kick;
            for (int i = 0; i < BAR_COUNT; ++i)
            {
                if (m_bars[i])
                {
                    m_bars[i]->m_transform.m_localTransform.m_position = m_basePositions[i] * factor;
                }
            }
        }
    } // namespace ui
} // namespace app
