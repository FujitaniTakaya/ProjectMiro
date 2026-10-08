/**
 * @file AIControllerConfig.h
 * @brief AIControllerの調整値
 * @details 既定値はProjectBeastのEnemyController / EnemyParameter.json の値。
 *          AIControllerを作るときに渡すので、キャラクターごとに違う値のAIを付けられる。
 *          移動速度(歩き/走り)は、操作される側のキャラクターが決めるので、ここには持たない。
 */
#pragma once


namespace app
{
    /**
     * @brief AIControllerの調整値
     */
    struct AIControllerConfig
    {
        AIControllerConfig()
            : sightDistance(600.0f)
            , sightHalfAngleDeg(70.0f)
            , maxDistFromHome(500.0f)
            , idleTimeMax(5.0f)
            , arriveDistWander(120.0f)
            , arriveDistReturnHome(120.0f)
            , stuckMoveThreshold(5.0f)
            , stuckTimeLimit(3.0f)
            , lostChaseDistance(700.0f)
            , maxChaseSeconds(30.0f)
            , attackDistance(80.0f)
            , arriveLastKnownDist(20.0f)
            , attackInterval(1.5f)
            , wanderRadius(300.0f)
            , eyeHeight(80.0f)
            , rayEndMargin(40.0f)
        {
        }


        /** 目標を見つけられる距離 */
        float sightDistance;
        /** 目標を見つけられる視野の半分の角度(度)。正面からこの角度までの範囲が見える */
        float sightHalfAngleDeg;
        /** ホームから離れすぎたと判定する距離。超えるとホームへ戻る */
        float maxDistFromHome;
        /** 待機する最大の時間(秒)。実際の待機時間は 0 ～ この値 のランダム */
        float idleTimeMax;
        /** 徘徊の目的地に着いたと判定する距離 */
        float arriveDistWander;
        /** ホームに着いたと判定する距離 */
        float arriveDistReturnHome;
        /** 停滞の判定: この距離以下しか動いていなければ停滞とみなす */
        float stuckMoveThreshold;
        /** 停滞の判定: この時間(秒)停滞し続けたら徘徊を諦める */
        float stuckTimeLimit;
        /** 追跡を諦める距離。目標がこれより離れるとホームへ戻る */
        float lostChaseDistance;
        /** 追跡し続けられる時間(秒)。尽きるとホームへ戻る。ホームに着くと回復する(BeastのスタミナのmaxStamina / 減少率) */
        float maxChaseSeconds;
        /** 攻撃する距離。この距離以内では止まって、Action1を押す */
        float attackDistance;
        /** 目標を見失ったとき、最後に見た位置に着いたと判定する距離 */
        float arriveLastKnownDist;
        /** 攻撃(Action1)を押す間隔(秒)。1フレームより長くすること(短いと押しっぱなしになり、Triggerが立たない) */
        float attackInterval;
        /** 巡回点がないとき、ホームから徘徊の目的地を決める最大の半径(実際は半径の 0.5 ～ 1.0 倍) */
        float wanderRadius;
        /** 視線の判定をする高さ(足元から)。足元だと地面に当たってしまうので持ち上げる */
        float eyeHeight;
        /** 視線の判定の両端を縮める距離。自分と目標のカプセルに当たらないように、カプセルの半径より大きくすること */
        float rayEndMargin;
    };
}
