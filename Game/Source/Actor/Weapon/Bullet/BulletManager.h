/**
 * @file BulletManager.h
 * @brief 弾の生成・更新・描画・破棄を行うクラス
 */
#pragma once
#include <cstddef>
#include <memory>
#include <vector>


namespace app
{
    namespace actor
    {
        class Bullet;


        /**
         * @brief 弾の生成・更新・描画・破棄を行うクラス
         * @details シングルトン。生きている弾を全部ここが持つ。外から Shot() を呼ぶと、弾を1発作って飛ばす。
         *          寿命が尽きた弾は、Update() の中で破棄する。
         *          例: BulletManager::Get().Shot(rotation, muzzlePosition, 3000.0f);   // 発射(どこからでも)
         *              BulletManager::Get().Update();       // シーンの Update()
         *              BulletManager::Get().Render(rc);     // シーンの Render()
         *              BulletManager::Get().Clear();        // シーンの終了時
         * @note 弾はモデル(エンジンの資源)を持つので、エンジンが終わる前に、シーンの終了時に Clear() を呼ぶこと。
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
             */
            void Shot(const Quaternion& rotation, const Vector3& position, const float speed);

            /**
             * @brief 更新処理
             * @details 全ての弾を進めてから、寿命が尽きた弾を破棄する。
             */
            void Update();

            /**
             * @brief 描画処理
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc);

            /**
             * @brief 全ての弾を破棄する
             * @details シーンの終了時に呼ぶ。
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
        };
    } // namespace actor
} // namespace app
