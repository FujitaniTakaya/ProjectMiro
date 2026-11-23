/**
 * @file ZombieSpawner.h
 * @brief ゾンビを湧かせるかどうか、どこに何を湧かせるかを決めるクラス
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "ZombieSpawnParameter.h"
#include "ZombieSpawnRule.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 湧かせる要求
         * @details ZombieSpawner が決めた、湧かせるゾンビの種類と座標。実際に作るのは ZombieManager。
         */
        struct ZombieSpawnRequest
        {
            /** 湧かせるゾンビの種類 */
            EnZombieKind kind;
            /** 湧かせる座標(ゲートの中心からずらして、浮かせた位置) */
            Vector3 position;
        };


        /**
         * @brief ゾンビを湧かせるかどうか、どこに何を湧かせるかを決めるクラス
         * @details 4つのゲートのどれかから、ゾンビを1〜3体ほどまとめて(グループで)湧かせる、という判断だけを行う。
         *          ゾンビが死んだ時に周りへアンガー値をばらまくので、同じ場所にまとまって湧かせる。
         *          ゾンビは作らないし、持たない。作って持つのは ZombieManager(このクラスはその中に入っている)。
         *          - グループ : 一定のインターバルごとに1グループ。数は groupMinCount〜groupMaxCount で、groupPeakCount が一番出やすい山なりの確率。
         *                       ゲートは4つからランダムに1つ選び、グループ全員が同じゲートから湧く。1フレームに1体ずつ、続けて湧かせる
         *          - 位置 : ゲートの中心から、体ごとに少しずらした位置に、少し浮かせて湧かせる
         *          - 種類 : 湧いた総数で決める。runningPeriod の倍数番目は runningChancePercent の確率で走るゾンビ、
         *                   runningGuaranteedPeriod の倍数番目は必ず走るゾンビ。他は通常のゾンビ
         *          - インターバル : intervalMinSeconds〜intervalMaxSeconds の整数秒。全部の値が出切るまで、同じ値は出ない。最初のグループは待たずに湧く
         *          - 上限 : maxAliveCount。生きている数がこれに達したら、グループの残りは湧かせずに捨てる
         *          Update() は、ZombieManager の Update() から毎フレーム呼ぶ。経過時間はこのクラスが自分で足すので、ポーズ中(Update()が呼ばれない間)は止まる。
         *          例: if (const std::optional<ZombieSpawnRequest> request = spawner.Update(aliveCount))
         *              {
         *                  // request->kind のゾンビを request->position に作る
         *              }
         * @note 1回の Update() で湧かせるのは1体まで。ゾンビ1体を作るとモデル・当たり判定の生成で負荷がかかるので、同じフレームにまとめて作らない。
         */
        class ZombieSpawner
        {
        public:
            ZombieSpawner();
            ~ZombieSpawner();


            /**
             * @brief 更新処理
             * @details タイマーを進めて、グループを湧かせ終えていて待ち時間も過ぎていれば、次のグループを始める。
             *          グループの途中なら、生きている数に空きがある間、1回の呼び出しにつき1体ぶんの湧かせる要求を返す。
             *          湧かせる要求を返した時は、これまでに湧かせた数が1つ増える。
             * @param aliveCount 今生きているゾンビの数(上限の判定に使う)
             * @return 湧かせる要求。今は湧かせない時は std::nullopt
             */
            std::optional<ZombieSpawnRequest> Update(const size_t aliveCount);

            /**
             * @brief これまでに湧かせた数を取得
             * @return 湧かせた数(Update() が要求を返した回数。外れたゾンビも含む)
             */
            inline uint32_t GetSpawnedCount() const
            {
                return m_spawnedCount;
            }


        private:
            /**
             * @brief 次のグループを始める
             * @details グループの数と、湧くゲートを決める。次のグループまでの待ち時間を袋から引いて、経過時間を0に戻す。
             */
            void StartGroup();


            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieSpawnParameter& m_parameter;
            /** 最後にグループを始めてからの経過時間(秒) */
            float m_elapsed;
            /** 次のグループを始めるまでの待ち時間(秒)。最初は0なので、最初のグループは待たずに湧く */
            float m_interval;
            /** インターバル(秒)の袋。全部出切るまで、同じ値は出ない */
            std::vector<int> m_intervalBag;
            /** 今のグループで、まだ湧かせていない数 */
            int m_pendingCount;
            /** 今のグループが湧くゲート */
            EnZombieGate m_groupGate;
            /** これまでに湧かせた数。走るゾンビの周期の判定に使う */
            uint32_t m_spawnedCount;
        };
    } // namespace actor
} // namespace app
