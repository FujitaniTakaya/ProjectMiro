/**
 * @file BulletHit.h
 * @brief 弾が当たった情報と、当たった時の処理
 */
#pragma once
#include <functional>


namespace app
{
    namespace actor
    {
        /**
         * @brief 弾が当たった情報
         */
        struct BulletHit
        {
            BulletHit()
                : object(nullptr)
                , position(Vector3::Zero)
                , point(Vector3::Zero)
                , normal(Vector3::Zero)
            {}


            /** 当たった物理オブジェクト(ゾンビの体・ステージ・コアの箱など)。ゾンビかどうかは、持ち主(シーン)が ZombieBase::IsBody() などで調べる */
            const btCollisionObject* object;
            /** 当たった時の球の中心。弾はここで止まる */
            Vector3 position;
            /** 当たった面の点。着弾の演出に使う */
            Vector3 point;
            /** 当たった面の法線 */
            Vector3 normal;
        };


        /**
         * @brief 弾が何かに当たった時の処理
         * @details 誰に当てるか・どうダメージを与えるかは、持ち主(シーン)が結びつけて渡す。(ZombieAttackHandler と同じ作り)
         *          ステージ・コアなど、何も起きない物に当たっても呼ばれる。その時は何もしなくてよい。
         *          設定していない(空の)処理は、何も起きないものとして扱われる。
         * @note 渡した処理が参照するものは、BulletManager::Clear() までの間、有効であること。
         * @param hit 当たった情報
         */
        using BulletHitHandler = std::function<void(const BulletHit& hit)>;
    } // namespace actor
} // namespace app
