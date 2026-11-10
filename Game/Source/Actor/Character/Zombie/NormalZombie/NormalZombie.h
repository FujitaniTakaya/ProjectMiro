/**
 * @file NormalZombie.h
 * @brief 通常のゾンビ
 */
#pragma once
#include "Source/Actor/Character/Zombie/ZombieBase.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 通常のゾンビ
         * @details 基本の1種類。パラメーターは EnParamID::Zombie(Zombie.json)、モデルは Assets/modelData/character/zombie/Zombie.tkm。
         *          動き・HP・アンガー値は ZombieBase のまま。種類固有なのは、パラメーターとモデルデータだけ。
         *          コントローラーは付けない。GetControllerSlot() から付けるまでは、無入力(待機)になる。
         * @note 初期座標は、Start() の前に SetPosition() で設定しておくこと。
         */
        class NormalZombie : public ZombieBase
        {
        public:
            NormalZombie();
            virtual ~NormalZombie() override;
        };
    } // namespace actor
} // namespace app
