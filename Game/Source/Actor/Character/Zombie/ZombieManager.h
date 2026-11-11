/**
 * @file ZombieManager.h
 * @brief ゾンビの生成・破棄・保持を行うクラス
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "Source/Actor/Character/Zombie/AI/ZombieCoreApproach.h"
#include "Source/Actor/Character/Zombie/Spawn/ZombieSpawnRule.h"
#include "Source/Actor/Character/Zombie/Spawn/ZombieSpawner.h"
#include "ZombieAttackHandler.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        class ZombieBase;
        struct ZombieSpawnParameter;


        /**
         * @brief ゾンビの生成・破棄・保持を行うクラス
         * @details 生きているゾンビを全部ここが持つ。作る・更新する・描画する・破棄するも、ここが行う。
         *          湧かせる判断(ゲート・間隔・種類)は、中の ZombieSpawner が行う。ここは、その結果(ZombieSpawnRequest)を受けて作るだけ。
         *          湧いたゾンビには、コアへ向かうControllerを付ける。
         *          HPが0以下になったゾンビは、Update() の頭で外して破棄する。
         *          Update() は、シーンの Update() から毎フレーム呼ぶ。ポーズ中(Update()が呼ばれない間)は、湧かせるタイマーも止まる。
         *          例: manager.SetCorePositionProvider(provider);
         *              manager.SetCoreAttackHandler(handler);
         *              manager.Preload();      // シーンのStart()
         *              manager.Update();       // シーンのUpdate()
         *              manager.Render(rc);     // シーンのRender()
         * @note 1回の Update() で湧かせるのは1体まで。ゾンビ1体を作るとモデル・当たり判定の生成で負荷がかかるので、同じフレームにまとめて作らない。
         *       破棄は Update() の中でだけ行う。(描画の途中でゾンビが無くならないように)
         *       シーンが持つこと。コアを参照する関数を渡すので、シーンのメンバーはコアより後に宣言して、コアより先に破棄されるようにする。
         *       ゾンビを外から指す方法(ハンドルなど)は、まだ無い。
         */
        class ZombieManager : public Noncopyable
        {
        public:
            /** コアの位置を教える関数(ZombieCoreApproach と同じ) */
            using PositionProvider = ZombieCoreApproach::PositionProvider;


        public:
            ZombieManager();
            ~ZombieManager();


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
             * @details 種類ごとに1体作って、すぐに捨てる。モデル・アニメーション・テクスチャなどは、一度読み込むとエンジンのバンクに残って使い回されるので、
             *          最初に湧かせる時の負荷(最初の1体だけ、1秒近くかかる)が無くなる。
             *          シーンの Start() など、画面が暗転している間に呼ぶこと。
             * @note Update() は呼ばないので、作ったゾンビは動かない。ゾンビを持つリストにも入れない。
             */
            void Preload();

            /**
             * @brief 更新処理
             * @details HPが0以下のゾンビを外す → スポーナーに聞いて、湧かせる時なら1体作る → 全ゾンビを更新する、の順。
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
                return m_spawner.GetSpawnedCount();
            }


        private:
            /**
             * @brief 指定した種類のゾンビを1体作る
             * @details 座標を設定して Start() まで行い、コアへ向かうControllerを付ける。Update() はまだ呼ばれていない。
             * @param kind ゾンビの種類
             * @param position 湧かせる座標
             * @return 作ったゾンビ
             */
            std::unique_ptr<ZombieBase> Create(const EnZombieKind kind, const Vector3& position) const;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える)。Preload() のゲート座標に使う */
            const ZombieSpawnParameter& m_parameter;
            /** コアの位置を教える関数。湧いたゾンビのControllerへ渡す */
            PositionProvider m_corePositionProvider;
            /** ゾンビの攻撃判定が重なった時の処理。湧いたゾンビへ渡す */
            ZombieAttackHandler m_coreAttackHandler;
            /** 湧かせる判断を行うもの */
            ZombieSpawner m_spawner;
            /** 生きているゾンビ */
            std::vector<std::unique_ptr<ZombieBase>> m_zombies;
        };
    } // namespace actor
} // namespace app
