/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <memory>

#include "Source/Actor/Character/Miro/Miro.h"


namespace app
{
    DebugScene::DebugScene()
    {
        m_miro = std::make_unique<actor::Miro>();
    }


    DebugScene::~DebugScene()
    {}


    bool DebugScene::Start()
    {
        m_miro->StartWrapper();
        return true;
    }


    void DebugScene::Update()
    {
        m_miro->UpdateWrapper();
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
