/**
 * @file DebugScene.cpp
 * @brief デバッグ用のシーン
 */
#include "stdafx.h"

#include "DebugScene.h"

#include <memory>

#include "Source/Actor/Character/Miro/Miro.h"
#include "Source/Camera/PlayerCamera.h"
#include "Source/Manager/InGameInfoHub.h"
#include "Source/UI/PlayerHP/HPBarMenu.h"


namespace app
{
    namespace
    {
        /** プレイヤーのHPバーのレイアウトのjson */
        constexpr const char* HP_BAR_JSON_PATH = "Assets/parameter/ui/HPBar.json";
    } // namespace


    DebugScene::DebugScene()
        : m_miro(nullptr)
        , m_playerCamera(nullptr)
        , m_hpBarPacket(nullptr)
    {
        // プレイヤーのHPバー。Miroより先に作って、InGameInfoHubに通知されたHPを受け取る
        ui::InitUIPacket(m_hpBarPacket, HP_BAR_JSON_PATH);
        if (ui::HPBarMenu* hpBarMenu = m_hpBarPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnPlayerHpChanged([hpBarMenu](const float hp, const float maxHp) { hpBarMenu->SetHp(hp, maxHp); });
        }

        m_miro = std::make_unique<actor::Miro>();
        m_playerCamera = std::make_unique<camera::PlayerCamera>();
    }


    DebugScene::~DebugScene()
    {
        // 登録した関数は、HPバーを参照している。HPバーが無くなる前に解除する
        InGameInfoHub::Get().ResetObservers();
    }


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

        // MiroがHPを通知した後に、HPバーを更新する
        m_hpBarPacket->Update();
    }


    void DebugScene::PauseUpdate()
    {}


    void DebugScene::Render(RenderContext& rc)
    {
        m_miro->RenderWrapper(rc);
    }


    void DebugScene::RenderUI(RenderContext& rc)
    {
        m_hpBarPacket->Render(rc);
    }


    bool DebugScene::RequestScene(uint32_t& id, float& waitTime)
    {
        return false;
    }
} // namespace app
