/**
 * @file RunningZombie.h
 * @brief 走り続けるゾンビ
 */
#pragma once
#include "Source/Actor/Character/Zombie/ZombieBase.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 走り続けるゾンビ
         * @details 特殊ゾンビの一種。アンガー値が溜まらない限りコアへ向かい続ける動きは NormalZombie と同じで、向かう動作が歩きではなく走り。
         *          パラメーターは EnParamID::RunningZombie(RunningZombie.json。構造体は ZombieParameter で、NormalZombie と同じ)、
         *          モデルは Assets/modelData/character/zombie/Zombie.tkm。(専用のモデルはまだ無いので、見た目は NormalZombie と同じ)
         *          動き・HP・アンガー値は ZombieBase のまま。種類固有なのは、パラメーターとモデルデータだけ。
         *          走るのは Controller が走りボタンを押すため。RunningZombieController を GetControllerSlot() から付けること。
         *          付けるまでは、無入力(待機)になる。
         * @note 走り続ける種類なので、普段の速さは ZombieParameter::runSpeed(ロックオン時の速さと同じ項目)。walkSpeed は使われない。
         *       初期座標は、Start() の前に SetPosition() で設定しておくこと。
         */
        class RunningZombie : public ZombieBase
        {
        public:
            RunningZombie();
            virtual ~RunningZombie() override;
        };
    } // namespace actor
} // namespace app
