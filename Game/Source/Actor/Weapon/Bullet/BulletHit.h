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
         * @brief ゾンビの頭の判定(ゴースト)に付ける属性
         * @details enCollisionAttr_User の次の値。弾のスイープは、この属性のゴーストにも当たる。(ゾンビの攻撃判定の箱など、属性の無いゴーストは素通りする)
         *          頭のゴーストは、持ち主(ゾンビ)を userPointer に持つ。体のカプセルも、同じ持ち主を userPointer に持つ。
         */
        constexpr int HEAD_COLLISION_ATTR = enCollisionAttr_User + 1;


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
                , isHead(false)
            {}


            /** 当たった物理オブジェクト(ゾンビの体・ステージ・コアの箱など)。ゾンビかどうかは、持ち主(シーン)が ZombieBase::IsBody() などで調べる。頭の球に当たった時は、その頭のゴースト */
            const btCollisionObject* object;
            /** 当たった時の球の中心。弾はここで止まる */
            Vector3 position;
            /** 当たった面の点。着弾の演出に使う */
            Vector3 point;
            /** 当たった面の法線 */
            Vector3 normal;
            /** 頭に当たったか。頭の球に当たった時と、体に当たって、弾の道筋がその持ち主の頭を通っていた時に true */
            bool isHead;
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
