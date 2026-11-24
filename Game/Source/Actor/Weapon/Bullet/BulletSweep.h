/**
 * @file BulletSweep.h
 * @brief 弾の当たり判定(球のスイープ)と、レティクルへ向かう発射角度の計算
 */
#pragma once
#include "BulletHit.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 球を from から to へ動かして、最初に当たる物を調べる
         * @details 物理ワールドのうち、次のものに当たる。
         *          - ステージ・ゾンビの体など、ゴースト以外の全て
         *          - ユーザー属性(enCollisionAttr_User)を付けたゴースト(コアの箱)
         *          次のものは無視する。
         *          - ignore(発射した人の体)
         *          - ユーザー属性の無いゴースト(ゾンビの攻撃判定の箱)
         * @note 球が最初から何かに重なっている場合の結果は、物理エンジンの動き次第。(壁にめり込んだ銃口からの発射は、呼ぶ側で別に調べる)
         * @param from 球の中心の始点
         * @param to 球の中心の終点
         * @param radius 球の半径(cm)
         * @param ignore 無視する物理オブジェクト。無視しないなら nullptr
         * @param[out] hit 当たった情報。当たらなかった時は変更されない
         * @return 当たったら true
         */
        bool SweepSphere(const Vector3& from, const Vector3& to, float radius, const btCollisionObject* ignore, BulletHit& hit);

        /**
         * @brief fromの向きをtoの向きへ回す回転を作る
         * @details Quaternion::SetRotation(from, to) は、約3.6度以内の差を「回さない」ことにしてしまう。
         *          発射の向きがその分ずれるので、ずれない版をここに持つ。
         * @param from 元の向き(大きさは問わない)
         * @param to 回した後の向き(大きさは問わない)
         * @return from を to へ回す回転
         */
        Quaternion MakeRotationFromTo(const Vector3& from, const Vector3& to);

        /**
         * @brief レティクル(カメラの視線)を伸ばして、最初に当たる物を調べる
         * @details 視線の始点は、銃口と同じ奥行きの面に置く。(カメラが後ろにいても、銃口より手前の物には当たらない)
         *          CalcAimRotation() の狙い点と、同じ視線。
         * @param cameraPosition カメラの位置
         * @param cameraForward カメラの前方向(大きさは問わない)
         * @param muzzle 銃口の位置
         * @param range 視線を伸ばす長さ(cm)。弾が飛べる距離にすること
         * @param radius 視線を調べる球の半径(cm)。弾の当たり判定と同じにすること
         * @param ignore 無視する物理オブジェクト(Miroの体)
         * @param[out] hit 当たった情報。当たらなかった時は変更されない
         * @param[out] endPoint 視線の終点(当たらなかった時の狙い点)。不要なら nullptr
         * @return 当たったら true
         */
        bool SweepAimRay(
            const Vector3& cameraPosition,
            const Vector3& cameraForward,
            const Vector3& muzzle,
            float range,
            float radius,
            const btCollisionObject* ignore,
            BulletHit& hit,
            Vector3* endPoint = nullptr
        );

        /**
         * @brief レティクル(カメラの視線)が指す点へ向かう、銃口からの発射角度を求める
         * @details カメラの視線を、銃口と同じ奥行きの面から伸ばして、最初に当たる点を狙い点にする。
         *          (カメラと銃口の間、Miroの後ろにある物には当たらないようにするため)
         *          何にも当たらなければ、視線の先 range の点を狙う。
         *          返す角度は、Vector3::Front を回すと銃口から狙い点への向きになるもの。(Bullet が進む向きの決め方と同じ)
         * @note 狙い点が銃口に近すぎる(壁に張り付いているなど)と、向きが不安定なので fallback を返す。
         * @param cameraPosition カメラの位置
         * @param cameraForward カメラの前方向(大きさは問わない)
         * @param muzzle 銃口の位置
         * @param range 視線を伸ばす長さ(cm)。弾が飛べる距離にすること
         * @param radius 視線を調べる球の半径(cm)。弾の当たり判定と同じにすること
         * @param ignore 無視する物理オブジェクト(Miroの体)
         * @param fallback 狙い点が使えない時に返す角度(銃口の向き)
         * @return 発射角度
         */
        Quaternion CalcAimRotation(
            const Vector3& cameraPosition,
            const Vector3& cameraForward,
            const Vector3& muzzle,
            float range,
            float radius,
            const btCollisionObject* ignore,
            const Quaternion& fallback
        );
    } // namespace actor
} // namespace app
