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

            // コアへ向かう間は、ずっと走りボタンを押す。止まっている間は押さない(待機になる)
            if (m_coreApproach.Approach(context, input))
            {
                input.SetPress(EnInputButton::ActionB);
            }
            return input;
        }
    } // namespace actor
} // namespace app
