/**
 * @file NormalZombieController.h
 * @brief 通常のゾンビ用のController
 * @details コアの位置へ一直線に歩く入力を作る。コアへ向かう処理は ZombieCoreApproach が行う。
 *          コアへ向かう間はボタンを押さないので、ゾンビは歩きで進む。コアに着いたら攻撃ボタン(ActionA)を押して、攻撃する。
 *          NormalZombie には付けていない。付けるときは、GetControllerSlot().Attach() で付ける。
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
         * @brief 通常のゾンビ用のController
         */
        class NormalZombieController : public IController
        {
        public:
            /** コアの位置を教える関数(ZombieCoreApproach と同じ) */
            using PositionProvider = ZombieCoreApproach::PositionProvider;


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
             * @details コアへ向かう間は、移動入力(長さ1)だけを作る。ボタンは押さない。
             *          コアに着いたら、移動入力はなく、攻撃ボタン(ActionA)だけを押す。
             * @param context 操作対象の情報。座標からコアへの向きを決める
             * @return 今フレームの入力。コアがなければ無入力
             */
            CharacterInput Update(const ControllerContext& context) override;


        private:
            /** コアへ向かう処理 */
            ZombieCoreApproach m_coreApproach;
        };
    } // namespace actor
} // namespace app
