/**
 * @file ZombieSpawner.cpp
 * @brief ゾンビを湧かせるかどうか、どこに何を湧かせるかを決めるクラス
 */
#include "stdafx.h"

#include "ZombieSpawner.h"

#include <cmath>

#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Util/RandomDevice.h"
#include "ZombieSpawnParameter.h"


namespace app
{
    namespace actor
    {
        ZombieSpawner::ZombieSpawner()
            : m_parameter(ParamHolder::Get().GetParameter<ZombieSpawnParameter>(EnParamID::ZombieSpawn))
            , m_elapsed(0.0f)
            , m_spawnedCount(0)
        {
        }


        ZombieSpawner::~ZombieSpawner() = default;


        std::optional<ZombieSpawnRequest> ZombieSpawner::Update(const size_t aliveCount)
        {
            // 湧かせる時間で、数にも空きがあれば、1体湧かせる
            m_elapsed = AdvanceSpawnTimer(m_elapsed, g_gameTime->GetFrameDeltaTime(), m_parameter.spawnInterval);
            if (!CanSpawn(m_elapsed, m_parameter.spawnInterval, aliveCount, m_parameter.maxAliveCount))
            {
                return std::nullopt;
            }
            m_elapsed = 0.0f;

            // ゲートを、4つからランダムに選ぶ
            const int gateIndex = util::RandomDevice::Random(0, static_cast<int>(EnZombieGate::Max) - 1);
            Vector3 position = GetGatePosition(m_parameter, static_cast<EnZombieGate>(gateIndex));

            // ゲートの中心から、円の中でずらす。(同じ点に湧くとゾンビ同士がぶつかって詰まる。半径の平方根をとって、円の中で一様にばらけさせる)
            const float angle = util::RandomDevice::Random(0.0f, Math::PI2);
            const float radius = m_parameter.gateJitterRadius * std::sqrt(util::RandomDevice::Random(0.0f, 1.0f));
            position.x += std::cos(angle) * radius;
            position.z += std::sin(angle) * radius;

            // 浮かせて湧かせる。高台のゲートでも、落ちて接地する
            position.y += m_parameter.dropHeight;

            const EnZombieKind kind = PickZombieKind(m_parameter.spawnOrder, m_spawnedCount);
            ++m_spawnedCount;
            return ZombieSpawnRequest{ kind, position };
        }
    } // namespace actor
} // namespace app
