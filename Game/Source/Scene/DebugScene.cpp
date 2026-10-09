/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"


namespace app
{
    DebugScene::DebugScene()
    {}


    DebugScene::~DebugScene()
    {}


    bool DebugScene::Start()
    {
        return true;
    }


    void DebugScene::Update()
    {}


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {}


    bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }
} // namespace app
