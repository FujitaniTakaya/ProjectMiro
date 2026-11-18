/**
 * @file InGameHud.cpp
 * @brief インゲームのHUD
 */
#include "stdafx.h"

#include "InGameHud.h"

#include <cstdint>

#include "Source/Manager/InGameInfoHub.h"
#include "Source/UI/HUD/CoreHP/CoreHPBarMenu.h"
#include "Source/UI/HUD/Handgun/HandgunAmmoMenu.h"
#include "Source/UI/HUD/PlayerHP/HPBarMenu.h"


namespace app
{
    namespace
    {
        /** プレイヤーのHPバーのレイアウトのjson */
        constexpr const char* HP_BAR_JSON_PATH = "Assets/parameter/ui/HPBar.json";
        /** コアの耐久値バーのレイアウトのjson */
        constexpr const char* CORE_HP_BAR_JSON_PATH = "Assets/parameter/ui/CoreHPBar.json";
        /** ハンドガンの残段数UIのレイアウトのjson */
        constexpr const char* HANDGUN_AMMO_JSON_PATH = "Assets/parameter/ui/HandgunAmmo.json";
        /** レティクルのレイアウトのjson */
        constexpr const char* RETICLE_JSON_PATH = "Assets/parameter/ui/Reticle.json";
    } // namespace


    InGameHud::InGameHud()
        : m_hpBarPacket(nullptr)
        , m_ammoPacket(nullptr)
        , m_reticlePacket(nullptr)
        , m_coreHpBarPacket(nullptr)
    {
        // プレイヤーのHPバー。Miroより先に作って、InGameInfoHubに通知されたHPを受け取る
        ui::InitUIPacket(m_hpBarPacket, HP_BAR_JSON_PATH);
        if (ui::HPBarMenu* hpBarMenu = m_hpBarPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnPlayerHpChanged([hpBarMenu](const float hp, const float maxHp) { hpBarMenu->SetHp(hp, maxHp); });
        }

        // ハンドガンの残段数UI。ハンドガンより先に作って、InGameInfoHubに通知された残弾数を受け取る
        ui::InitUIPacket(m_ammoPacket, HANDGUN_AMMO_JSON_PATH);
        if (ui::HandgunAmmoMenu* ammoMenu = m_ammoPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnPlayerAmmoChanged([ammoMenu](const uint32_t remainingAmmo) { ammoMenu->SetAmmo(remainingAmmo); });
        }

        ui::InitUIPacket(m_reticlePacket, RETICLE_JSON_PATH);

        // コアの耐久値バー。コアより先に作って、InGameInfoHubに通知された耐久値を受け取る
        ui::InitUIPacket(m_coreHpBarPacket, CORE_HP_BAR_JSON_PATH);
        if (ui::CoreHPBarMenu* coreHpBarMenu = m_coreHpBarPacket->GetMenu())
        {
            InGameInfoHub::Get().SetOnCoreHpChanged([coreHpBarMenu](const float hp, const float maxHp) { coreHpBarMenu->SetHp(hp, maxHp); });
        }
    }


    InGameHud::~InGameHud()
    {
        // 登録した関数は、各Menuを参照している。Menuが無くなる前に解除する
        InGameInfoHub::Get().ResetObservers();
    }


    void InGameHud::SetCoreHpBarWorldPosition(const Vector3& worldPosition)
    {
        if (ui::CoreHPBarMenu* coreHpBarMenu = m_coreHpBarPacket->GetMenu())
        {
            coreHpBarMenu->UpdateScreenPosition(worldPosition);
        }
    }


    void InGameHud::Update()
    {
        m_hpBarPacket->Update();
        m_coreHpBarPacket->Update();
        m_ammoPacket->Update();
        m_reticlePacket->Update();
    }


    void InGameHud::Render(RenderContext& rc)
    {
        m_hpBarPacket->Render(rc);
        m_coreHpBarPacket->Render(rc);
        m_ammoPacket->Render(rc);
        m_reticlePacket->Render(rc);
    }
} // namespace app
