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

            // コアへ向かう間は、ボタンを押さない。移動入力だけで、ゾンビは歩きになる。
            // コアに着いたら、攻撃ボタンを押す。(コアがないときは押さない)
            if (!m_coreApproach.Approach(context, input) && m_coreApproach.IsArrived(context))
            {
                input.SetPress(EnInputButton::ActionA);
            }
            return input;
        }
    } // namespace actor
} // namespace app
