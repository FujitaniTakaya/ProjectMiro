/**
 * @file NormalZombieController.h
 * @brief 通常のゾンビ用のController
 * @details コアの位置へ一直線に歩く入力を作る。コアの位置は SetCorePositionProvider() で教える。
 *          コアまでのXZ平面の距離が ZombieParameter::coreStopDistance 以内になったら、着いたとみなして立ち止まる。
 *          ボタンは押さないので、ゾンビは歩きで進む。
 *          NormalZombie には付けていない。付けるときは、GetControllerSlot().Attach() で付ける。
 * @note 経路探索がまだ無いので、間に壁や崖があっても真っ直ぐ向かう。
 *       アンガー値が溜まり切ったとき(本格ロックオン)の動きもまだ無いので、アンガー値は見ずに、常にコアへ向かう。
 */
#pragma once
#include <functional>

#include "Source/MoveBehavior/IController.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct ZombieParameter;


        /**
         * @brief 通常のゾンビ用のController
         */
        class NormalZombieController : public IController
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
            explicit NormalZombieController(const ZombieParameter& parameter);


            /**
             * @brief コアの位置を教える関数を設定する
             * @note コアを破棄する前に nullptr を設定すること(関数が持っているポインタが残らないように)
             * @param provider コアの位置を教える関数。nullptr ならコアなし(無入力)
             */
            void SetCorePositionProvider(const PositionProvider& provider);


            /**
             * @brief 今フレームの入力を作る
             * @details コアが着いたとみなす距離より遠ければ、コアへ向かう移動入力(長さ1)を作る。
             * @param context 操作対象の情報。座標からコアへの向きを決める
             * @return 今フレームの入力。コアがない、またはコアに着いていれば無入力
             */
            CharacterInput Update(const ControllerContext& context) override;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieParameter& m_parameter;
            /** コアの位置を教える関数 */
            PositionProvider m_corePositionProvider;
        };
    } // namespace actor
} // namespace app
