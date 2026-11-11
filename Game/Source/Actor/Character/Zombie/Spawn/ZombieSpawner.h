/**
 * @file ZombieSpawner.h
 * @brief ゾンビを湧かせるかどうか、どこに何を湧かせるかを決めるクラス
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>

#include "ZombieSpawnRule.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct ZombieSpawnParameter;


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
         * @details 4つのゲートのどれかから、一定の間隔でゾンビを1体ずつ湧かせる、という判断だけを行う。
         *          ゾンビは作らないし、持たない。作って持つのは ZombieManager(このクラスはその中に入っている)。
         *          - ゲート : 4つからランダムに選ぶ。ゲートの中心から、少しずらした位置に、少し浮かせて湧かせる
         *          - 種類 : ZombieSpawnParameter::spawnOrder の順(例: NNNR = 3体歩き→1体走りの繰り返し)
         *          - 間隔・上限 : ZombieSpawnParameter の spawnInterval・maxAliveCount。生きている数が上限の間は湧かない
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
             * @details タイマーを進めて、湧かせる時間で、生きている数にも空きがあれば、湧かせる要求を返す。
             *          湧かせる要求を返した時は、これまでに湧かせた数が1つ増え、タイマーが0に戻る。
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
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieSpawnParameter& m_parameter;
            /** 最後に湧かせてからの経過時間(秒)。湧かせる間隔で止まる */
            float m_elapsed;
            /** これまでに湧かせた数。湧く順番のどこを使うかを決める */
            uint32_t m_spawnedCount;
        };
    } // namespace actor
} // namespace app
