/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <memory>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Camera/PlayerCamera.h"


namespace app
{
    DebugScene::DebugScene()
    {
        m_miro = std::make_unique<actor::Miro>();
        m_playerCamera = std::make_unique<camera::PlayerCamera>();
    }


    DebugScene::~DebugScene()
    {}


    bool DebugScene::Start()
    {
        m_miro->StartWrapper();
        m_playerCamera->Start();
        return true;
    }


    void DebugScene::Update()
    {
        m_miro->UpdateWrapper();

        // Miroが動いた後の座標を追う
        m_playerCamera->SetTargetPosition(m_miro->GetTransform().m_position);
        m_playerCamera->Update();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
        m_miro->RenderWrapper(rc);
    }


    bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }
} // namespace app
