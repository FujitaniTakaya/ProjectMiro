/**
 * @file ControllerSlot.cpp
 * @brief Controllerを付け替えられるようにする入れ物
 */
#include "stdafx.h"
#include "ControllerSlot.h"


namespace app
{
    ControllerSlot::ControllerSlot()
        : m_controller(nullptr)
        , m_input()
        , m_isUpdating(false)
    {
    }


    ControllerSlot::~ControllerSlot()
    {
        // 付いているControllerに外れることを知らせる
        Detach();
    }


    void ControllerSlot::Attach(std::unique_ptr<IController> controller)
    {
        K2_ASSERT(!m_isUpdating, "Controllerの更新中に付け替えることはできません\n");
        if (m_isUpdating)
        {
            return;
        }

        // 付いているControllerを外す
        if (m_controller)
        {
            m_controller->OnDetach();
            m_controller.reset();
        }

        // 新しいControllerを付ける(入力は無入力に戻す)
        m_controller = std::move(controller);
        m_input = CharacterInput();
        if (m_controller)
        {
            m_controller->OnAttach();
        }
    }


    std::unique_ptr<IController> ControllerSlot::Detach()
    {
        K2_ASSERT(!m_isUpdating, "Controllerの更新中に外すことはできません\n");
        if (m_isUpdating)
        {
            return nullptr;
        }

        if (m_controller)
        {
            m_controller->OnDetach();
        }
        m_input = CharacterInput();
        return std::move(m_controller);
    }


    const CharacterInput& ControllerSlot::Update(const ControllerContext& context)
    {
        // Controllerが付いていなければ無入力
        CharacterInput next;
        if (m_controller)
        {
            ControllerContext updateContext = context;
            updateContext.deltaTime = g_gameTime->GetFrameDeltaTime();

            m_isUpdating = true;
            next = m_controller->Update(updateContext);
            m_isUpdating = false;
        }

        // 前フレームとの差から、押した瞬間を計算する
        next.UpdateTrigger(m_input);
        m_input = next;

        return m_input;
    }
}
