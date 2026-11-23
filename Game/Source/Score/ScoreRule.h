/**
 * @file ScoreRule.h
 * @brief スコアの計算規則
 * @details リザルトで出すスコアを、守衛時間と倒したゾンビの数から計算する。
 *          エンジンに依存しない関数だけを置く(単体でテストできるように、シーン・UIから分けてある)。
 *          単位は「守衛1秒 = 1点」。ゾンビの点も、この秒の単位に合わせた小さな整数で、桁上げの倍率は使わない。
 *          - 守衛時間 : 1秒 = 1点
 *          - 通常ゾンビ : 1体 = 2点
 *          - ランニングゾンビ : 1体 = 6点(通常の3倍。走り 0.9 / 歩き 0.3 = 3倍の速さで、コアに着くまでの猶予が1/3のため)
 *          湧き順 NNNR・3秒間隔(ZombieSpawn.json)で全部倒すと、12秒あたり 時間12点 + 撃破12点で、時間と撃破が半々になる。
 *          点を変えたいときは、DEFAULT_SCORE_WEIGHTS を直す。
 */
#pragma once
#include <cstdint>


namespace app
{
    /**
     * @brief 1要素あたりの点
     */
    struct ScoreWeights
    {
        /** 守衛時間1秒あたりの点 */
        int pointsPerSecond;
        /** 通常ゾンビ1体あたりの点 */
        int normalKillPoints;
        /** ランニングゾンビ1体あたりの点 */
        int runningKillPoints;
    };


    /** 既定の点。守衛1秒 = 1点、通常ゾンビ = 2点、ランニングゾンビ = 6点 */
    constexpr ScoreWeights DEFAULT_SCORE_WEIGHTS = { 1, 2, 6 };


    /**
     * @brief スコアの内訳
     */
    struct ScoreBreakdown
    {
        ScoreBreakdown()
            : timePoints(0)
            , normalPoints(0)
            , runningPoints(0)
            , total(0)
        {}

        /** 守衛時間の点 */
        int timePoints;
        /** 通常ゾンビを倒した点 */
        int normalPoints;
        /** ランニングゾンビを倒した点 */
        int runningPoints;
        /** 合計 */
        int total;
    };


    /**
     * @brief スコアを計算する
     * @details 守衛時間の端数は切り捨てる。(画面に出す「m:ss」も切り捨てなので、表示と点が合う)
     * @param survivedSeconds 守衛継続時間(秒)。負の値とNaNは0として扱う
     * @param normalKills 倒した通常ゾンビの数
     * @param runningKills 倒したランニングゾンビの数
     * @param weights 1要素あたりの点
     * @return スコアの内訳
     */
    inline ScoreBreakdown CalcScore(const float survivedSeconds, const uint32_t normalKills, const uint32_t runningKills, const ScoreWeights& weights = DEFAULT_SCORE_WEIGHTS)
    {
        // NaNも0にするため、「0より大きい」の否定で判定する。正の値のintへのキャストは切り捨て
        const int seconds = survivedSeconds > 0.0f ? static_cast<int>(survivedSeconds) : 0;

        ScoreBreakdown breakdown;
        breakdown.timePoints = seconds * weights.pointsPerSecond;
        breakdown.normalPoints = static_cast<int>(normalKills) * weights.normalKillPoints;
        breakdown.runningPoints = static_cast<int>(runningKills) * weights.runningKillPoints;
        breakdown.total = breakdown.timePoints + breakdown.normalPoints + breakdown.runningPoints;
        return breakdown;
    }
} // namespace app
