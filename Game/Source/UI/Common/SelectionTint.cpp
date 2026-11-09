/**
 * @file SelectionTint.cpp
 * @brief 選んでいない選択肢の色を、薄灰色にする
 */
#include "stdafx.h"

#include "SelectionTint.h"

#include <algorithm>
#include <cstddef>


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 選んでいない選択肢の色に掛ける倍率(RGB)。白が約178の薄灰色になる */
            constexpr float DIM_RATE = 0.7f;
            /** 選んでいる選択肢が切り替わる(明るさが0から1に動く)のにかける時間(秒) */
            constexpr float FADE_TIME = 0.1f;
        } // namespace


        SelectionTint::SelectionTint()
            : m_parts()
            , m_amounts()
        {}


        SelectionTint::~SelectionTint() = default;


        void SelectionTint::Clear()
        {
            m_parts.clear();
            m_amounts.clear();
        }


        void SelectionTint::AddPart(const int choice, UIBase* ui)
        {
            if (!ui || choice < 0)
            {
                return;
            }

            m_parts.push_back({ ui, ui->m_color, choice });

            // 増えた分の選択肢は、元の色から始める。(登録の直後にSnap()かStep()で、選んでいる状態に合わせる)
            const size_t size = static_cast<size_t>(choice) + 1;
            if (m_amounts.size() < size)
            {
                m_amounts.resize(size, 1.0f);
            }
        }


        void SelectionTint::Step(const int selected, const float deltaTime)
        {
            const float step = deltaTime / FADE_TIME;
            for (size_t i = 0; i < m_amounts.size(); ++i)
            {
                const bool isSelected = (static_cast<int>(i) == selected);
                float& amount = m_amounts[i];
                amount = isSelected ? (std::min)(amount + step, 1.0f) : (std::max)(amount - step, 0.0f);
            }

            Apply();
        }


        void SelectionTint::Snap(const int selected)
        {
            for (size_t i = 0; i < m_amounts.size(); ++i)
            {
                m_amounts[i] = (static_cast<int>(i) == selected) ? 1.0f : 0.0f;
            }

            Apply();
        }


        void SelectionTint::Apply()
        {
            for (const Part& part : m_parts)
            {
                const float rate = DIM_RATE + (1.0f - DIM_RATE) * m_amounts[static_cast<size_t>(part.m_choice)];
                part.m_ui->m_color.x = part.m_baseColor.x * rate;
                part.m_ui->m_color.y = part.m_baseColor.y * rate;
                part.m_ui->m_color.z = part.m_baseColor.z * rate;
            }
        }
    } // namespace ui
} // namespace app
