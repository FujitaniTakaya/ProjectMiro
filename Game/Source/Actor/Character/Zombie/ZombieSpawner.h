/**
 * @file ZombieSpawner.h
 * @brief ゾンビを湧かせて、コアへ向かわせるクラス
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "ZombieAttackHandler.h"
#include "ZombieCoreApproach.h"
#include "ZombieSpawnRule.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        class ZombieBase;
        struct ZombieSpawnParameter;


        /**
         * @brief ゾンビを湧かせて、コアへ向かわせるクラス
         * @details 4つのゲートのどれかから、一定の間隔でゾンビを1体ずつ湧かせる。湧いたゾンビには、コアへ向かうControllerを付ける。
         *          湧かせたゾンビは、このクラスが持つ。更新・描画・破棄も、このクラスが行う。
         *          - ゲート : 4つからランダムに選ぶ。ゲートの中心から、少しずらした位置に、少し浮かせて湧かせる
         *          - 種類 : ZombieSpawnParameter::spawnOrder の順(例: NNNR = 3体歩き→1体走りの繰り返し)
         *          - 間隔・上限 : ZombieSpawnParameter の spawnInterval・maxAliveCount。生きている数が上限の間は湧かない
         *          HPが0以下になったゾンビは、Update() の頭で外して破棄する。
         *          Update() は、シーンの Update() から毎フレーム呼ぶ。経過時間はこのクラスが自分で足すので、ポーズ中(Update()が呼ばれない間)は止まる。
         *          例: spawner.SetCorePositionProvider(provider);
         *              spawner.Update();       // シーンのUpdate()
         *              spawner.Render(rc);     // シーンのRender()
         * @note 1回の Update() で湧かせるのは1体まで。ゾンビ1体を作るとモデル・当たり判定の生成で負荷がかかるので、同じフレームにまとめて作らない。
         *       破棄は Update() の中でだけ行う。(描画の途中でゾンビが無くならないように)
         *       経路探索が無いので、ゲートからコアまで障害物があっても、ゾンビは真っ直ぐ向かう。
         */
        class ZombieSpawner
        {
        public:
            /** コアの位置を教える関数(ZombieCoreApproach と同じ) */
            using PositionProvider = ZombieCoreApproach::PositionProvider;


        public:
            ZombieSpawner();
            ~ZombieSpawner();


            /**
             * @brief コアの位置を教える関数を設定する
             * @details 設定した後に湧いたゾンビへ渡される。湧いている最中のゾンビには渡されないので、湧かせ始める前に設定すること。
             * @note コアを破棄する前に、このクラスを破棄するか、nullptr を設定すること(関数が持っているポインタが残らないように)
             * @param provider コアの位置を教える関数。nullptr ならコアなし(ゾンビは動かない)
             */
            void SetCorePositionProvider(const PositionProvider& provider);

            /**
             * @brief ゾンビの攻撃判定が何かに重なった時の処理を設定する
             * @details 設定した後に湧いたゾンビへ渡される。湧いている最中のゾンビには渡されないので、湧かせ始める前に設定すること。
             *          コアにダメージを与えるには、重なった判定がコアの箱なら Core::TakeDamage() を呼んで true を返す処理を渡す。
             * @note コアを破棄する前に、このクラスを破棄するか、空の処理を設定すること(処理が持っているポインタが残らないように)
             * @param handler 重なった時の処理。空なら何にも当たらない
             */
            void SetCoreAttackHandler(const ZombieAttackHandler& handler);

            /**
             * @brief ゾンビのモデルなどを、先に読み込んでおく
             * @details 種類ごとに1体作って、すぐに捨てる。モデル・アニメーション・テクスチャなどは、一度読み込むと使い回されるので、
             *          最初に湧かせる時の負荷(最初の1体だけ、1秒近くかかる)が無くなる。
             *          シーンの Start() など、画面が暗転している間に呼ぶこと。
             */
            void Preload();

            /**
             * @brief 更新処理
             * @details HPが0以下のゾンビを外す → タイマーを進めて、湧かせる時間なら1体湧かせる → 全ゾンビを更新する、の順。
             *          湧いたゾンビは、同じ Update() の中で最初の更新が行われる。(描画の前にモデルの位置が確定する)
             */
            void Update();

            /**
             * @brief 描画処理
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc);

            /**
             * @brief 生きているゾンビの数を取得
             * @return 今このクラスが持っているゾンビの数
             */
            inline size_t GetAliveCount() const
            {
                return m_zombies.size();
            }

            /**
             * @brief これまでに湧かせた数を取得
             * @return 湧かせた数(外れたゾンビも含む)
             */
            inline uint32_t GetSpawnedCount() const
            {
                return m_spawnedCount;
            }


        private:
            /**
             * @brief ゾンビを1体湧かせる
             * @details ゲートと位置をランダムに決め、湧く順番から種類を決めて、作る。作ったゾンビは m_zombies に入れる。
             */
            void SpawnOne();

            /**
             * @brief 指定した種類のゾンビを1体作る
             * @details 座標を設定して Start() まで行い、コアへ向かうControllerを付ける。Update() はまだ呼ばれていない。
             * @param kind ゾンビの種類
             * @param position 湧かせる座標
             * @return 作ったゾンビ
             */
            std::unique_ptr<ZombieBase> Create(const EnZombieKind kind, const Vector3& position) const;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieSpawnParameter& m_parameter;
            /** コアの位置を教える関数。湧いたゾンビのControllerへ渡す */
            PositionProvider m_corePositionProvider;
            /** ゾンビの攻撃判定が重なった時の処理。湧いたゾンビへ渡す */
            ZombieAttackHandler m_coreAttackHandler;
            /** 生きているゾンビ */
            std::vector<std::unique_ptr<ZombieBase>> m_zombies;
            /** 最後に湧かせてからの経過時間(秒)。湧かせる間隔で止まる */
            float m_elapsed;
            /** これまでに湧かせた数。湧く順番のどこを使うかを決める */
            uint32_t m_spawnedCount;
        };
    } // namespace actor
} // namespace app
