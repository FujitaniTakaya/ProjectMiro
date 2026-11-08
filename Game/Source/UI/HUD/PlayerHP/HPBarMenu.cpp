/**
 * @file HPBarMenu.cpp
 * @brief HPバーを動かすMenu
 */
#include "stdafx.h"

#include "HPBarMenu.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Source/Core/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 赤のバーのUIの名前(ハッシュ値) */
            constexpr uint32_t RED_GAUGE_KEY = Hash32("RedGauge");
            /** 緑のバーのUIの名前(ハッシュ値) */
            constexpr uint32_t GREEN_GAUGE_KEY = Hash32("GreenGauge");

            /** 赤のバーが緑のバーに追いつく速さ(1秒あたり)。60fpsで、1フレームに残りの約8%ずつ縮まる */
            constexpr float RED_CHASE_RATE = 5.0f;
            /** この差より小さくなったら、赤のバーを緑のバーに揃える */
            constexpr float RED_SNAP_DISTANCE = 0.001f;
        } // namespace


        HPBarMenu::HPBarMenu()
            : m_redGauge(nullptr)
            , m_greenGauge(nullptr)
            , m_greenRatio(1.0f)
            , m_redRatio(1.0f)
        {}


        HPBarMenu::~HPBarMenu() = default;


        void HPBarMenu::Update()
        {
            if (m_redRatio > m_greenRatio)
            {
                // 緑のバーに向かって、時間をかけて縮む(フレームレートに依らない)
                const float deltaTime = g_gameTime->GetFrameDeltaTime();
                m_redRatio += (m_greenRatio - m_redRatio) * (1.0f - std::exp(-RED_CHASE_RATE * deltaTime));
                if (m_redRatio - m_greenRatio < RED_SNAP_DISTANCE)
                {
                    m_redRatio = m_greenRatio;
                }
            }
            else
            {
                // 回復した時は、すぐ揃える(赤のバーは緑のバーの下に隠れる)
                m_redRatio = m_greenRatio;
            }

            // NOTE: Layoutがjsonの更新でUIのパーツを作り直すと、拡大率がjsonの値に戻る。毎フレーム書いて、すぐ直す。
            ApplyRatio();

            MenuBase::Update();
        }


        void HPBarMenu::InitializeLogic()
        {
            m_redGauge = GetUI<UIGauge>(RED_GAUGE_KEY);
            m_greenGauge = GetUI<UIGauge>(GREEN_GAUGE_KEY);

            if (!m_redGauge)
            {
                K2_LOG("HPBarMenu: UIGauge \"RedGauge\" がありません。\n");
            }
            if (!m_greenGauge)
            {
                K2_LOG("HPBarMenu: UIGauge \"GreenGauge\" がありません。\n");
            }

            ApplyRatio();
        }


        void HPBarMenu::SetHp(const float hp, const float maxHp)
        {
            m_greenRatio = (maxHp > 0.0f) ? std::clamp(hp / maxHp, 0.0f, 1.0f) : 0.0f;
        }


        void HPBarMenu::ApplyRatio()
        {
            if (m_redGauge)
            {
                m_redGauge->m_transform.m_localTransform.m_scale.x = m_redRatio;
            }
            if (m_greenGauge)
            {
                m_greenGauge->m_transform.m_localTransform.m_scale.x = m_greenRatio;
            }
        }
    } // namespace ui
} // namespace app
