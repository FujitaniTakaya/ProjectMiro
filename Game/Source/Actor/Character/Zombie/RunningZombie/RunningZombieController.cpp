/**
 * @file RunningZombieController.cpp
 * @brief 走り続けるゾンビ用のController
 */
#include "stdafx.h"

#include "RunningZombieController.h"


namespace app
{
    namespace actor
    {
        RunningZombieController::RunningZombieController(const ZombieParameter& parameter)
            : m_coreApproach(parameter)
        {
        }


        void RunningZombieController::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_coreApproach.SetCorePositionProvider(provider);
        }


        CharacterInput RunningZombieController::Update(const ControllerContext& context)
        {
            CharacterInput input;

            // コアへ向かう間は、ずっと走りボタンを押す。
            // コアに着いたら、走りボタンの代わりに攻撃ボタンを押す。(コアがないときは、どちらも押さない)
            if (m_coreApproach.Approach(context, input))
            {
                input.SetPress(EnInputButton::ActionB);
            }
            else if (m_coreApproach.IsArrived(context))
            {
                input.SetPress(EnInputButton::ActionA);
            }
            return input;
        }
    } // namespace actor
} // namespace app
