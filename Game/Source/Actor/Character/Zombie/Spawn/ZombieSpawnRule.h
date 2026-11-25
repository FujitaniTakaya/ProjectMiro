/**
 * @file ZombieSpawnRule.h
 * @brief ゾンビを湧かせる規則
 * @details ZombieSpawner と ZombieManager が使う、どのゾンビをいつ湧かせるか・いつ外すかの規則。
 *          エンジンに依存しない関数だけを置く(単体でテストできるように、ZombieSpawner・ZombieManager から分けてある)。
 */
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
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
         * @brief 走るゾンビの判定の種類
         */
        enum class EnRunningCheck : uint8_t
        {
            /** 歩きのゾンビ(判定しない) */
            Never,
            /** 確率で走るゾンビになる(確率の判定は呼び出し側で行う) */
            Chance,
            /** 必ず走るゾンビになる */
            Always
        };


        /**
         * @brief 湧いた総数から、次に湧くゾンビの走る判定の種類を決める
         * @details 何体目か(1始まり)が guaranteedPeriod の倍数なら Always、period の倍数なら Chance、どちらでもなければ Never。
         *          両方の倍数のとき(例: 周期4と20の、20体目)は、Always が優先される。
         * @param spawnedCount これまでに湧かせた数。0なら、次に湧くのは1体目
         * @param period 判定をする周期。0以下なら判定しない
         * @param guaranteedPeriod 必ず走る周期。0以下なら無効
         * @return 判定の種類
         */
        inline EnRunningCheck GetRunningCheck(const uint32_t spawnedCount, const int period, const int guaranteedPeriod)
        {
            const uint32_t number = spawnedCount + 1;
            if (guaranteedPeriod > 0 && number % static_cast<uint32_t>(guaranteedPeriod) == 0)
            {
                return EnRunningCheck::Always;
            }
            if (period > 0 && number % static_cast<uint32_t>(period) == 0)
            {
                return EnRunningCheck::Chance;
            }
            return EnRunningCheck::Never;
        }


        /**
         * @brief グループの数ごとの重みを取得する
         * @details 中心(peakCount)が一番重く、離れるほど軽くなる山なりの重み。一番遠い端でも1以上。
         *          例: 範囲4〜8・中心6なら、4:1 5:2 6:3 7:2 8:1。
         *          minCount <= peakCount <= maxCount に丸めてから渡すこと。
         * @param minCount グループの最小数
         * @param maxCount グループの最大数
         * @param peakCount 一番出やすい数
         * @param count 重みを知りたい数
         * @return 重み(1以上)
         */
        inline int GetGroupCountWeight(const int minCount, const int maxCount, const int peakCount, const int count)
        {
            const int farthest = std::max<int>(peakCount - minCount, maxCount - peakCount);
            return std::max<int>(1, farthest + 1 - std::abs(count - peakCount));
        }


        /**
         * @brief minCount〜maxCount の重みの合計を取得する
         * @details PickGroupCount() に渡す乱数の範囲(0以上、この値未満)になる。
         * @param minCount グループの最小数
         * @param maxCount グループの最大数
         * @param peakCount 一番出やすい数
         * @return 重みの合計
         */
        inline int GetGroupCountTotalWeight(const int minCount, const int maxCount, const int peakCount)
        {
            int total = 0;
            for (int count = minCount; count <= maxCount; ++count)
            {
                total += GetGroupCountWeight(minCount, maxCount, peakCount, count);
            }
            return total;
        }


        /**
         * @brief 重みをつけて、グループの数を選ぶ
         * @details 小さい数から順に、重みの幅を roll から引いていき、収まった数を返す。
         * @param minCount グループの最小数
         * @param maxCount グループの最大数
         * @param peakCount 一番出やすい数
         * @param roll 0以上、GetGroupCountTotalWeight() 未満の乱数
         * @return 選ばれたグループの数(minCount〜maxCount)
         */
        inline int PickGroupCount(const int minCount, const int maxCount, const int peakCount, int roll)
        {
            for (int count = minCount; count <= maxCount; ++count)
            {
                const int weight = GetGroupCountWeight(minCount, maxCount, peakCount, count);
                if (roll < weight)
                {
                    return count;
                }
                roll -= weight;
            }
            return maxCount;
        }


        /**
         * @brief グループ間のインターバル(秒)を、袋から1つ取り出す
         * @details 袋が空なら、minSeconds〜maxSeconds の整数を全部入れ直す。そこからランダムに1つ選んで、袋から外して返す。
         *          袋の中身を全部出し切るまで、同じ値は出ない。入れ直した直後は、直前と同じ値が出てもよい。
         *          maxSeconds が minSeconds より小さいときは、minSeconds だけを入れる。
         * @tparam TRandomInt randomInt(lo, hi) で、lo以上hi以下の整数を返す関数(乱数の実装に依存しないよう、呼び出し側から渡す)
         * @param[in,out] bag 残りの値の袋
         * @param minSeconds インターバルの最小(秒)
         * @param maxSeconds インターバルの最大(秒)
         * @param randomInt 整数の乱数を返す関数
         * @return 取り出したインターバル(秒)
         */
        template <typename TRandomInt>
        int TakeIntervalFromBag(std::vector<int>& bag, const int minSeconds, const int maxSeconds, TRandomInt&& randomInt)
        {
            if (bag.empty())
            {
                for (int seconds = minSeconds; seconds <= std::max<int>(minSeconds, maxSeconds); ++seconds)
                {
                    bag.push_back(seconds);
                }
            }

            const int index = randomInt(0, static_cast<int>(bag.size()) - 1);
            const int seconds = bag[static_cast<size_t>(index)];
            bag.erase(bag.begin() + index);
            return seconds;
        }


        /**
         * @brief ずらし量が壁の側を向いていたら、内側へ折り返す
         * @details 内側の向き(inward)との内積が負なら、その成分を反転する。内側を向いていれば何もしない。
         *          円の中で一様なずらし量は、内側の半円の中で一様になる。
         * @param[in,out] offsetX ずらし量のX
         * @param[in,out] offsetZ ずらし量のZ
         * @param inwardX ゲートからステージの内側へ向かう単位ベクトルのX
         * @param inwardZ 同じくZ
         */
        inline void FoldOffsetInward(float& offsetX, float& offsetZ, const float inwardX, const float inwardZ)
        {
            const float along = offsetX * inwardX + offsetZ * inwardZ;
            if (along < 0.0f)
            {
                offsetX -= 2.0f * along * inwardX;
                offsetZ -= 2.0f * along * inwardZ;
            }
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
