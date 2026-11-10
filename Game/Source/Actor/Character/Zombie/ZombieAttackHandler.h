/**
 * @file ZombieAttackHandler.h
 * @brief ゾンビの攻撃判定が何かに重なった時の処理
 * @details ゾンビは、誰に当てるかを知らない。当たったかどうかとダメージの与え方は、持ち主(シーン)が結びつけて渡す。
 *          例: spawner.SetCoreAttackHandler([core](const btCollisionObject& contact, const float damage) { ... });
 */
#pragma once
#include <functional>


namespace app
{
    namespace actor
    {
        /**
         * @brief ゾンビの攻撃判定が何かに重なった時の処理
         * @details 攻撃判定が有効な間、毎フレーム、重なっている物を1つずつ渡す。
         *          渡された物が当てたい相手なら、ダメージを与えて true を返すこと。(その振りでは、以後は渡されない)
         *          当てたい相手ではない(地面・他のゾンビなど)なら、何もせず false を返すこと。
         *          設定していない(空の)処理は、何にも当たらないものとして扱われる。
         * @note 渡した処理が参照するものは、ゾンビが生きている間、有効であること。
         * @param contact 攻撃判定に重なっている物理ワールドの判定
         * @param damage 与えるダメージ(ZombieParameter の attackPower)
         * @return 当たりとして扱ったら true
         */
        using ZombieAttackHandler = std::function<bool(const btCollisionObject& contact, float damage)>;
    } // namespace actor
} // namespace app
