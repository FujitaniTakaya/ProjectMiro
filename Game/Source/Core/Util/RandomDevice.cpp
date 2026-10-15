/**
 * @file RandomDevice.cpp
 * @brief 乱数を生成するクラス
 */
#include "stdafx.h"

#include "RandomDevice.h"

#include <algorithm>


namespace app
{
    namespace util
    {
        namespace
        {
            /** パーセントの最小値 */
            constexpr float PERCENT_MIN = 0.0f;
            /** パーセントの最大値 */
            constexpr float PERCENT_MAX = 100.0f;
        } // namespace


        int RandomDevice::Random(const int min, const int max)
        {
            K2_ASSERT(min <= max, "minがmaxより大きい。");
            std::uniform_int_distribution<int> dist(min, max);
            return dist(GetEngine());
        }


        float RandomDevice::Random(const float min, const float max)
        {
            K2_ASSERT(min <= max, "minがmaxより大きい。");
            std::uniform_real_distribution<float> dist(min, max);
            return dist(GetEngine());
        }


        bool RandomDevice::Percent(const float percent)
        {
            const float clamped = std::clamp(percent, PERCENT_MIN, PERCENT_MAX);
            std::bernoulli_distribution dist(clamped / PERCENT_MAX);
            return dist(GetEngine());
        }


        void RandomDevice::Seed(const uint32_t seed)
        {
            GetEngine().seed(seed);
        }


        std::mt19937& RandomDevice::GetEngine()
        {
            // 関数内staticにして、他のstaticな変数の初期化中に呼ばれても、初期化済みであることを保証する
            static std::mt19937 engine{ std::random_device{}() };
            return engine;
        }
    } // namespace util
} // namespace app
