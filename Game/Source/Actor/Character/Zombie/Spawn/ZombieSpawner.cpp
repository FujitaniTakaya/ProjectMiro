/**
 * @file ZombieSpawner.cpp
 * @brief ゾンビを湧かせるかどうか、どこに何を湧かせるかを決めるクラス
 */
#include "stdafx.h"

#include "ZombieSpawner.h"

#include <algorithm>
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
            , m_interval(0.0f)
            , m_intervalBag()
            , m_pendingCount(0)
            , m_groupGate(EnZombieGate::North)
            , m_spawnedCount(0)
        {
        }


        ZombieSpawner::~ZombieSpawner() = default;


        void ZombieSpawner::StartGroup()
        {
            // 数の範囲は、min <= peak <= max に丸めてから使う
            const int minCount = std::max<int>(1, m_parameter.groupMinCount);
            const int maxCount = std::max<int>(minCount, m_parameter.groupMaxCount);
            const int peakCount = std::clamp(m_parameter.groupPeakCount, minCount, maxCount);

            // 山なりの重みで、グループの数を選ぶ
            const int totalWeight = GetGroupCountTotalWeight(minCount, maxCount, peakCount);
            m_pendingCount = PickGroupCount(minCount, maxCount, peakCount, util::RandomDevice::Random(0, totalWeight - 1));

            // ゲートを、4つからランダムに選ぶ。グループ全員が同じゲートから湧く
            m_groupGate = static_cast<EnZombieGate>(util::RandomDevice::Random(0, static_cast<int>(EnZombieGate::Max) - 1));

            // 次のグループまでの待ち時間を、袋から引く
            m_interval = static_cast<float>(TakeIntervalFromBag(
                m_intervalBag,
                m_parameter.intervalMinSeconds,
                m_parameter.intervalMaxSeconds,
                [](const int lo, const int hi) { return util::RandomDevice::Random(lo, hi); }
            ));
            m_elapsed = 0.0f;
        }


        std::optional<ZombieSpawnRequest> ZombieSpawner::Update(const size_t aliveCount)
        {
            m_elapsed += g_gameTime->GetFrameDeltaTime();

            // 前のグループを湧かせ終えていて、待ち時間が過ぎていれば、次のグループを始める
            if (m_pendingCount == 0 && m_elapsed >= m_interval)
            {
                StartGroup();
            }
            if (m_pendingCount == 0)
            {
                return std::nullopt;
            }

            // 上限に達していたら、グループの残りは捨てる(空きがある分だけ湧いたことになる)
            if (m_parameter.maxAliveCount <= 0 || aliveCount >= static_cast<size_t>(m_parameter.maxAliveCount))
            {
                m_pendingCount = 0;
                return std::nullopt;
            }
            // 1回の呼び出しで1体だけ。残りは次のフレーム以降に湧かせる
            --m_pendingCount;

            Vector3 position = GetGatePosition(m_parameter, m_groupGate);

            // ゲートの中心から、円の中でずらす。(同じ点に湧くとゾンビ同士がぶつかって詰まる。半径の平方根をとって、円の中で一様にばらけさせる)
            // ただし壁の側へはずらさず、内側へ折り返す。(壁に食い込んで湧くと、動けなくなる)
            const float angle = util::RandomDevice::Random(0.0f, Math::PI2);
            const float radius = m_parameter.gateJitterRadius * std::sqrt(util::RandomDevice::Random(0.0f, 1.0f));
            float offsetX = std::cos(angle) * radius;
            float offsetZ = std::sin(angle) * radius;
            const Vector3 inward = GetGateInward(m_groupGate);
            FoldOffsetInward(offsetX, offsetZ, inward.x, inward.z);
            position.x += offsetX;
            position.z += offsetZ;

            // 浮かせて湧かせる。高台のゲートでも、落ちて接地する
            position.y += m_parameter.dropHeight;

            // 種類は、湧いた総数で決める。周期の倍数番目は確率で走り、さらに大きな周期の倍数番目は必ず走る
            EnZombieKind kind = EnZombieKind::Normal;
            switch (GetRunningCheck(m_spawnedCount, m_parameter.runningPeriod, m_parameter.runningGuaranteedPeriod))
            {
            case EnRunningCheck::Always:
                kind = EnZombieKind::Running;
                break;
            case EnRunningCheck::Chance:
                if (util::RandomDevice::Percent(m_parameter.runningChancePercent))
                {
                    kind = EnZombieKind::Running;
                }
                break;
            case EnRunningCheck::Never:
            default:
                break;
            }
            ++m_spawnedCount;
            return ZombieSpawnRequest{ kind, position };
        }
    } // namespace actor
} // namespace app
