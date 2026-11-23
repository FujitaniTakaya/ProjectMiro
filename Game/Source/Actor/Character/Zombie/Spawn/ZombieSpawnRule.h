/**
 * @file ZombieSpawnRule.h
 * @brief ゾンビを湧かせる規則
 * @details ZombieSpawner と ZombieManager が使う、どのゾンビをいつ湧かせるか・いつ外すかの規則。
 *          エンジンに依存しない関数だけを置く(単体でテストできるように、ZombieSpawner・ZombieManager から分けてある)。
 */
#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <vector>


namespace app
{
    namespace actor
    {
        /**
         * @brief 湧かせるゾンビの種類
         */
        enum class EnZombieKind : uint8_t
        {
            /** 通常のゾンビ(歩き)。NormalZombie */
            Normal,
            /** 走り続けるゾンビ。RunningZombie */
            Running,

            Max
        };


        /**
         * @brief 湧く順番から、次に湧かせるゾンビの種類を決める
         * @details order の文字を、左から順に使う。最後の文字まで来たら先頭に戻る。
         *          R(r)は走り続けるゾンビ。それ以外の文字は通常のゾンビ。order が空なら、常に通常のゾンビ。
         *          例: "NNNR" なら、通常・通常・通常・走り・通常・通常・通常・走り…の順になる。
         * @param order 湧く順番(ZombieSpawnParameter::spawnOrder)
         * @param spawnedCount これまでに湧かせた数。0なら先頭の文字
         * @return 湧かせるゾンビの種類
         */
        inline EnZombieKind PickZombieKind(const std::string& order, const uint32_t spawnedCount)
        {
            if (order.empty())
            {
                return EnZombieKind::Normal;
            }

            const char symbol = order[spawnedCount % order.size()];
            return (symbol == 'R' || symbol == 'r') ? EnZombieKind::Running : EnZombieKind::Normal;
        }


        /**
         * @brief 湧かせるまでのタイマーを進める
         * @details 経過時間は interval で止まる。同時数が上限の間に時間を溜め込まないため。
         *          interval が負の値のときは、0として扱う。
         * @param elapsed 前のフレームまでの経過時間(秒)
         * @param deltaTime このフレームの時間(秒)
         * @param interval 湧かせる間隔(秒)
         * @return 進めた後の経過時間(0以上interval以下)
         */
        inline float AdvanceSpawnTimer(const float elapsed, const float deltaTime, const float interval)
        {
            return std::min<float>(elapsed + deltaTime, std::max<float>(interval, 0.0f));
        }


        /**
         * @brief 今、1体湧かせてよいか
         * @details 経過時間が間隔に達していて、生きている数が上限未満なら、湧かせてよい。上限が0以下なら、湧かせない。
         * @param elapsed 経過時間(秒)
         * @param interval 湧かせる間隔(秒)
         * @param aliveCount 生きているゾンビの数
         * @param maxAliveCount 同時に生きていられる数の上限
         * @return 湧かせてよければ true
         */
        inline bool CanSpawn(const float elapsed, const float interval, const size_t aliveCount, const int maxAliveCount)
        {
            if (maxAliveCount <= 0)
            {
                return false;
            }

            return elapsed >= interval && aliveCount < static_cast<size_t>(maxAliveCount);
        }


        /**
         * @brief 死亡の動きが終わったゾンビを外す
         * @details 外れたゾンビは破棄される。他のゾンビの順番は変わらない。HPが0になっても、死亡アニメーションが終わるまでは残る。
         *          外す前に、外すゾンビ1体ごとに onRemove を呼ぶ。(倒した数を種類別に数えるなど。外した後は破棄されて触れない)
         * @tparam TZombie ゾンビの型。IsDeathFinished() を持つこと
         * @tparam TOnRemove 外すゾンビを受け取る関数の型。const TZombie& を1つ取る
         * @param[in,out] zombies ゾンビのリスト
         * @param onRemove 外すゾンビ1体ごとに、外す前に呼ぶ関数
         * @return 外した数
         */
        template <typename TZombie, typename TOnRemove>
        size_t RemoveDeadZombies(std::vector<std::unique_ptr<TZombie>>& zombies, TOnRemove&& onRemove)
        {
            for (const std::unique_ptr<TZombie>& zombie : zombies)
            {
                if (zombie->IsDeathFinished())
                {
                    onRemove(*zombie);
                }
            }

            const auto newEnd = std::remove_if(zombies.begin(), zombies.end(), [](const std::unique_ptr<TZombie>& zombie) { return zombie->IsDeathFinished(); });
            const size_t removedCount = static_cast<size_t>(std::distance(newEnd, zombies.end()));
            zombies.erase(newEnd, zombies.end());
            return removedCount;
        }
    } // namespace actor
} // namespace app
