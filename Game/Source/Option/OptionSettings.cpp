/**
 * @file OptionSettings.cpp
 * @brief オプション画面で変える設定の値
 */
#include "stdafx.h"

#include "OptionSettings.h"

#include <algorithm>
#include <cmath>


namespace app
{
    namespace
    {
        /**
         * @brief 感度を範囲に収めて、刻みの倍数に丸める
         * @param sensitivity 感度(倍率)
         * @return 丸めた感度
         */
        float NormalizeSensitivity(const float sensitivity)
        {
            const float clamped = std::clamp(sensitivity, OptionSettings::MIN_SENSITIVITY, OptionSettings::MAX_SENSITIVITY);

            // 刻みの倍数に丸める。足し引きを繰り返しても、誤差が溜まらない。
            const float steps = std::round(clamped / OptionSettings::SENSITIVITY_STEP);
            return std::clamp(steps * OptionSettings::SENSITIVITY_STEP, OptionSettings::MIN_SENSITIVITY, OptionSettings::MAX_SENSITIVITY);
        }
    } // namespace


    OptionSettings::OptionSettings()
        : m_cameraSensitivity(DEFAULT_SENSITIVITY)
        , m_aimSensitivity(DEFAULT_SENSITIVITY)
    {}


    void OptionSettings::SetCameraSensitivity(const float sensitivity)
    {
        m_cameraSensitivity = NormalizeSensitivity(sensitivity);
    }


    void OptionSettings::SetAimSensitivity(const float sensitivity)
    {
        m_aimSensitivity = NormalizeSensitivity(sensitivity);
    }


    OptionSettings& OptionSettings::Get()
    {
        static OptionSettings instance;
        return instance;
    }
} // namespace app
