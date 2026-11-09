/**
 * @file MenuInput.cpp
 * @brief メニューを選ぶための方向入力
 */
#include "stdafx.h"

#include "MenuInput.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /**
             * @brief 十字キーと左スティックから、倒している方向を求める
             * @param isNegative 負の向き(左/上)の十字キーを押しているか
             * @param isPositive 正の向き(右/下)の十字キーを押しているか
             * @param stickTilt スティックの倒し具合(-1〜1)。負の向き(左/上)に倒すと負の値、正の向き(右/下)に倒すと正の値
             * @return -1、0、+1
             */
            int ResolveHeld(const bool isNegative, const bool isPositive, const float stickTilt)
            {
                if (isNegative != isPositive)
                {
                    return isPositive ? 1 : -1;
                }

                if (stickTilt >= MenuInput::STICK_TILT_THRESHOLD)
                {
                    return 1;
                }
                if (stickTilt <= -MenuInput::STICK_TILT_THRESHOLD)
                {
                    return -1;
                }
                return 0;
            }


            /**
             * @brief 倒し続けた時間までに、連続して返した回数を求める
             * @param holdTime 倒し続けた時間(秒)
             * @return 回数。REPEAT_DELAY を過ぎるまでは0
             */
            int CountRepeat(const float holdTime)
            {
                if (holdTime < MenuInput::REPEAT_DELAY)
                {
                    return 0;
                }
                return 1 + static_cast<int>((holdTime - MenuInput::REPEAT_DELAY) / MenuInput::REPEAT_INTERVAL);
            }
        } // namespace


        MenuInput::Axis::Axis()
            : m_held(0)
            , m_holdTime(0.0f)
            , m_move(0)
        {}


        void MenuInput::Axis::Update(const int held, const float deltaTime, const bool isRepeat)
        {
            m_move = 0;

            if (held == 0)
            {
                m_held = 0;
                m_holdTime = 0.0f;
                return;
            }

            // 倒し始めた瞬間(逆向きに倒し替えた時も)
            if (held != m_held)
            {
                m_held = held;
                m_holdTime = 0.0f;
                m_move = held;
                return;
            }

            // 倒し続けている間
            if (isRepeat)
            {
                const int before = CountRepeat(m_holdTime);
                m_holdTime += deltaTime;
                if (CountRepeat(m_holdTime) > before)
                {
                    m_move = held;
                }
            }
        }


        MenuInput::MenuInput()
            : m_x()
            , m_y()
        {}


        void MenuInput::Update(const GamePad& pad, const float deltaTime)
        {
            // 左スティックの上は、Yが正の値。上を-1にそろえるために、向きを逆にして渡す。
            const int heldX = ResolveHeld(pad.IsPress(enButtonLeft), pad.IsPress(enButtonRight), pad.GetLStickXF());
            const int heldY = ResolveHeld(pad.IsPress(enButtonUp), pad.IsPress(enButtonDown), -pad.GetLStickYF());
            UpdateDirection(heldX, heldY, deltaTime);
        }


        void MenuInput::UpdateDirection(const int heldX, const int heldY, const float deltaTime)
        {
            m_x.Update(heldX, deltaTime, true);
            m_y.Update(heldY, deltaTime, false);
        }


        void MenuInput::Reset()
        {
            m_x = Axis();
            m_y = Axis();
        }


        const GamePad* MenuInput::FindPad(const int padIndex)
        {
            if (padIndex < 0 || padIndex >= static_cast<int>(g_pad.size()) || g_pad[padIndex] == nullptr)
            {
                return nullptr;
            }

            return g_pad[padIndex];
        }
    } // namespace ui
} // namespace app
