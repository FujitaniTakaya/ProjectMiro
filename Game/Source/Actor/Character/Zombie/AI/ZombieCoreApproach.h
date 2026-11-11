/**
 * @file ZombieCoreApproach.h
 * @brief ゾンビがコアへ向かう処理
 * @details ゾンビの種類(NormalZombie・RunningZombieなど)のControllerが共通で使う、「コアへ向かう移動入力を作る」部品。
 *          コアの位置は SetCorePositionProvider() で教える。
 *          コアまでのXZ平面の距離が ZombieParameter::coreStopDistance 以内になったら、着いたとみなして何もしない。
 *          歩くか走るかは、この部品は決めない。移動入力だけを作り、ボタン(走りなど)は使う側のControllerが押す。
 * @note 経路探索の格子(NavigationService)ができていれば、その経路のウェイポイントに沿って向かう。
 *       経路から外れた・進めなくなったときは、探し直す。格子が無い・経路が見つからないときは、真っ直ぐ向かう。
 */
#pragma once
#include <functional>
#include <optional>

#include "Source/MoveBehavior/CharacterInput.h"
#include "Source/MoveBehavior/ControllerContext.h"
#include "Source/Navigation/PathFollower.h"


namespace app
{
    namespace nav
    {
        /** 前方宣言 */
        struct NavigationParameter;
    } // namespace nav


    namespace actor
    {
        /** 前方宣言 */
        struct ZombieParameter;


        /**
         * @brief ゾンビがコアへ向かう処理
         */
        class ZombieCoreApproach
        {
        public:
            /**
             * @brief コアの位置を教える関数
             * @return コアの足元(地面)のワールド座標。高さは使わない。コアがなければ std::nullopt
             */
            using PositionProvider = std::function<std::optional<Vector3>()>;


        public:
            /**
             * @brief コンストラクタ
             * @param parameter 付けるゾンビの種類のパラメーター(ParamHolderが持つもの。アプリが終わるまで使える)
             */
            explicit ZombieCoreApproach(const ZombieParameter& parameter);


            /**
             * @brief コアの位置を教える関数を設定する
             * @note コアを破棄する前に nullptr を設定すること(関数が持っているポインタが残らないように)
             * @param provider コアの位置を教える関数。nullptr ならコアなし
             */
            void SetCorePositionProvider(const PositionProvider& provider);


            /**
             * @brief コアへ向かう移動入力を作る
             * @details コアが着いたとみなす距離より遠ければ、コアへ向かう移動入力(長さ1)を input に設定する。
             *          高さは無視して、XZ平面で向かう。
             *          経路探索の格子があれば、経路のウェイポイントへ向かう。経路が無い・古いときは、ここで探す(探し直しは repathInterval の間隔)。
             *          格子が無い・経路が見つからないときは、コアへ真っ直ぐ向かう。
             * @param context 操作対象の情報。座標からコアへの向きを決める
             * @param[out] input 移動入力を設定する先。向かわないときは変更しない
             * @return コアへ向かう入力を設定したら true。コアがない・コアに着いている・真上や真下にいて向きが決まらないときは false
             */
            bool Approach(const ControllerContext& context, CharacterInput& input);

            /**
             * @brief コアに着いているかどうか
             * @details コアへ向かうのをやめる距離(coreStopDistance 以内、または真上・真下)にいるかどうか。Approach() が false を返す理由のうち、「着いている」だけを調べる。
             *          着いたあとの動き(攻撃など)を始めるかどうかを、使う側のControllerが決めるのに使う。
             * @param context 操作対象の情報。座標からコアまでの距離を測る
             * @return コアがあって、着いていれば true。コアがなければ false
             */
            bool IsArrived(const ControllerContext& context) const;


        private:
            /**
             * @brief コアへの向きと、着いているかどうか
             */
            struct CoreApproachInfo
            {
                /** コアの足元のワールド座標 */
                Vector3 corePosition;
                /** 座標からコアへの水平方向(XZ)のベクトル。長さは正規化していない */
                Vector3 toCore;
                /** 着いたとみなす距離以内なら true */
                bool isArrived;
            };


        private:
            /**
             * @brief コアへの向きと、着いているかどうかを求める
             * @param context 操作対象の情報
             * @return コアへの向きと、着いているかどうか。コアがなければ std::nullopt
             */
            std::optional<CoreApproachInfo> CalcToCore(const ControllerContext& context) const;

            /**
             * @brief 経路に沿って向かう水平方向を求める
             * @details 経路をたどる状況を進めて、経路が無い・終わった・外れた・進めなくなったときは、探し直す(間隔は repathInterval)。
             * @param context 操作対象の情報
             * @param corePosition コアの足元のワールド座標
             * @return 向かう水平方向(長さ1)。Yは0。格子が無い・経路が見つからないときは std::nullopt
             */
            std::optional<Vector3> CalcPathDirection(const ControllerContext& context, const Vector3& corePosition);


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieParameter& m_parameter;
            /** 経路探索のパラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const nav::NavigationParameter& m_navParameter;
            /** コアの位置を教える関数 */
            PositionProvider m_corePositionProvider;
            /** 経路をたどる処理 */
            nav::PathFollower m_pathFollower;
            /** 次に経路を探してよくなるまでの残り時間(秒) */
            float m_repathCooldown;
        };
    } // namespace actor
} // namespace app
