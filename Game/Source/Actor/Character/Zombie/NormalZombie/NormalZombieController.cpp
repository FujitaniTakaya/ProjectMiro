/**
 * @file NormalZombieController.cpp
 * @brief 通常のゾンビ用のController
 */
#include "stdafx.h"

#include "NormalZombieController.h"


namespace app
{
    namespace actor
    {
        NormalZombieController::NormalZombieController(const ZombieParameter& parameter)
            : m_coreApproach(parameter)
        {
        }


        void NormalZombieController::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_coreApproach.SetCorePositionProvider(provider);
        }


        CharacterInput NormalZombieController::Update(const ControllerContext& context)
        {
            CharacterInput input;

            // ボタンは押さない。移動入力だけで、ゾンビは歩きになる
            m_coreApproach.Approach(context, input);
            return input;
        }
    } // namespace actor
} // namespace app
