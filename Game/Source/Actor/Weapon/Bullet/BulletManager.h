/**
 * @file BulletManager.h
 * @brief 弾の生成・更新・描画・破棄を行うクラス
 */
#pragma once
#include <cstddef>
#include <memory>
#include <vector>

#include "BulletHit.h"


namespace app
{
    namespace actor
    {
        class Bullet;


        /**
         * @brief 弾の生成・更新・描画・破棄を行うクラス
         * @details シングルトン。生きている弾を全部ここが持つ。外から Shot() を呼ぶと、弾を1発作って飛ばす。
         *          何かに当たった弾・寿命が尽きた弾は、Update() の中で破棄する。弾が当たった時は、SetHitHandler() で渡された処理を呼ぶ。
         *          例: BulletManager::Get().SetHitHandler(handler);   // シーンの Start()。誰に当てるかはシーンが決める
         *              BulletManager::Get().Shot(rotation, muzzlePosition, 3000.0f, selfBody);   // 発射(どこからでも)
         *              BulletManager::Get().Update();       // シーンの Update()
         *              BulletManager::Get().Render(rc);     // シーンの Render()
         *              BulletManager::Get().Clear();        // シーンの終了時
         * @note 弾はモデル(エンジンの資源)を持つので、エンジンが終わる前に、シーンの終了時に Clear() を呼ぶこと。
         *       当たった時の処理はシーンのポインタを持つことがあるので、Clear() で外れる。
         *       Update() をシーンから呼ぶので、ポーズ中(Update() が呼ばれない間)は弾も止まる。
         *       破棄は Update() の中でだけ行う。(描画の途中で弾が無くならないように)
         */
        class BulletManager : public Noncopyable
        {
        private:
            BulletManager();
            ~BulletManager();


        public:
            /**
             * @brief インスタンスを取得する
             * @details 初めて呼んだときに作られ、プロセスの終了時に破棄される。生成・破棄の呼び出しは要らない。
             * @return インスタンス
             */
            static BulletManager& Get();

            /**
             * @brief 弾を1発撃つ
             * @details 弾を作って、すぐにモデルを読み込む。次の Update() から飛ぶ。
             * @param rotation 発射角度。この回転で Vector3::Front を回した向きに進む
             * @param position 発射位置
             * @param speed 発射スピード(cm/秒)
             * @param ignore 弾が当たらない物理オブジェクト(発射した人の体)。無ければ nullptr
             */
            void Shot(const Quaternion& rotation, const Vector3& position, const float speed, const btCollisionObject* ignore = nullptr);

            /**
             * @brief 弾が当たった時の処理を設定する
             * @details 弾が何かに当たったフレームに、1回呼ばれる。Clear() で外れる。
             * @param handler 当たった時の処理。空なら何も起きない
             */
            void SetHitHandler(const BulletHitHandler& handler);

            /**
             * @brief 弾を出さずに、その場で着弾として扱う
             * @details 銃口が壁やゾンビの中にある時など、弾を出す前に当たりが分かった時に使う。当たった時の処理が呼ばれる。
             * @param hit 当たった情報
             */
            void NotifyHit(const BulletHit& hit);

            /**
             * @brief 更新処理
             * @details 全ての弾を進めて、当たった弾について当たった時の処理を呼んでから、当たった弾・寿命が尽きた弾を破棄する。
             */
            void Update();

            /**
             * @brief 描画処理
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc);

            /**
             * @brief 全ての弾を破棄する
             * @details シーンの終了時に呼ぶ。当たった時の処理も外す。
             */
            void Clear();

            /**
             * @brief 飛んでいる弾の数を取得
             * @return 今このクラスが持っている弾の数
             */
            inline size_t GetBulletCount() const
            {
                return m_bullets.size();
            }


        private:
            /** 飛んでいる弾 */
            std::vector<std::unique_ptr<Bullet>> m_bullets;
            /** 弾が当たった時の処理 */
            BulletHitHandler m_hitHandler;
        };
    } // namespace actor
} // namespace app
