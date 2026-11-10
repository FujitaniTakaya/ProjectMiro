/**
 * @file ZombieCoreApproach.h
 * @brief ゾンビがコアへ向かう処理
 * @details ゾンビの種類(NormalZombie・RunningZombieなど)のControllerが共通で使う、「コアへ向かう移動入力を作る」部品。
 *          コアの位置は SetCorePositionProvider() で教える。
 *          コアまでのXZ平面の距離が ZombieParameter::coreStopDistance 以内になったら、着いたとみなして何もしない。
 *          歩くか走るかは、この部品は決めない。移動入力だけを作り、ボタン(走りなど)は使う側のControllerが押す。
 * @note 経路探索がまだ無いので、間に壁や崖があっても真っ直ぐ向かう。
 */
#pragma once
#include <functional>

#include "Source/MoveBehavior/CharacterInput.h"
#include "Source/MoveBehavior/ControllerContext.h"


namespace app
{
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
             * @param outPosition コアの足元(地面)のワールド座標を入れる。高さは使わない
             * @return コアがあれば true。なければ false
             */
            using PositionProvider = std::function<bool(Vector3& outPosition)>;


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
             * @param context 操作対象の情報。座標からコアへの向きを決める
             * @param[out] input 移動入力を設定する先。向かわないときは変更しない
             * @return コアへ向かう入力を設定したら true。コアがない・コアに着いている・真上や真下にいて向きが決まらないときは false
             */
            bool Approach(const ControllerContext& context, CharacterInput& input) const;

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
             * @brief コアへの向きと、着いているかどうかを求める
             * @param context 操作対象の情報
             * @param[out] outToCore 座標からコアへの水平方向(XZ)のベクトル。長さは正規化していない
             * @param[out] outIsArrived 着いたとみなす距離以内なら true
             * @return コアがあって求められたら true。なければ false(出力は変更しない)
             */
            bool CalcToCore(const ControllerContext& context, Vector3& outToCore, bool& outIsArrived) const;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieParameter& m_parameter;
            /** コアの位置を教える関数 */
            PositionProvider m_corePositionProvider;
        };
    } // namespace actor
} // namespace app
