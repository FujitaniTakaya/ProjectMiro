/**
 * @file RunningZombieController.h
 * @brief 走り続けるゾンビ用のController
 * @details コアの位置へ一直線に走る入力を作る。コアへ向かう処理は ZombieCoreApproach が行う(NormalZombieController と同じ)。
 *          違いは、コアへ向かっている間、ずっと走りボタン(ActionB)を押すこと。ゾンビは走りで進む。コアに着いたら立ち止まる。
 *          RunningZombie には付けていない。付けるときは、GetControllerSlot().Attach() で付ける。
 * @note アンガー値が溜まり切ったとき(本格ロックオン)の動きもまだ無いので、アンガー値は見ずに、常にコアへ向かう。
 */
#pragma once
#include "Source/Actor/Character/Zombie/ZombieCoreApproach.h"
#include "Source/MoveBehavior/IController.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct ZombieParameter;


        /**
         * @brief 走り続けるゾンビ用のController
         */
        class RunningZombieController : public IController
        {
        public:
            /** コアの位置を教える関数(ZombieCoreApproach と同じ) */
            using PositionProvider = ZombieCoreApproach::PositionProvider;


        public:
            /**
             * @brief コンストラクタ
             * @param parameter 付けるゾンビの種類のパラメーター(ParamHolderが持つもの。アプリが終わるまで使える)
             */
            explicit RunningZombieController(const ZombieParameter& parameter);


            /**
             * @brief コアの位置を教える関数を設定する
             * @note コアを破棄する前に nullptr を設定すること(関数が持っているポインタが残らないように)
             * @param provider コアの位置を教える関数。nullptr ならコアなし(無入力)
             */
            void SetCorePositionProvider(const PositionProvider& provider);


            /**
             * @brief 今フレームの入力を作る
             * @details コアへ向かう移動入力(長さ1)と、走りボタン(ActionB)を作る。
             *          止まっている間(コアに着いた・コアがない)は、ボタンも押さない。
             * @param context 操作対象の情報。座標からコアへの向きを決める
             * @return 今フレームの入力。コアがない、またはコアに着いていれば無入力
             */
            CharacterInput Update(const ControllerContext& context) override;


        private:
            /** コアへ向かう処理 */
            ZombieCoreApproach m_coreApproach;
        };
    } // namespace actor
} // namespace app
