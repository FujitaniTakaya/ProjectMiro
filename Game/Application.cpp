/**
 * @file Application.cpp
 * @brief アプリケーション全体を管理するクラス
 */
#include "stdafx.h"

#include "Application.h"

#include "Source/Core/HotReload/HotReloadManager.h"
#include "Source/Core/Param/ParamDebugUI.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Timer/TimerManager.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Font/FontManager.h"
#include "Source/Scene/Fade.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundDebugUI.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
    Application::Application()
        : m_isDrawCollision(false)
    {
        SoundManager::CreateInstance();
        EffectManager::CreateInstance();
        ParamHolder::CreateInstance();
        FontManager::CreateInstance();
        Fade::CreateInstance();
        SceneManager::CreateInstance();

        m_soundDebugUI = std::make_unique<SoundDebugUI>();
        m_paramDebugUI = std::make_unique<ParamDebugUI>();
    }


    Application::~Application()
    {
        m_paramDebugUI.reset();
        m_soundDebugUI.reset();

        SceneManager::DestroyInstance();
        Fade::DestroyInstance();
        FontManager::DestroyInstance();
        ParamHolder::DestroyInstance();
        EffectManager::DestroyInstance();
        SoundManager::DestroyInstance();
    }


    void Application::PreUpdate()
    {
        HotReloadManager::Get().Update();
        SoundManager::Get().Update();
        EffectManager::Get().Update();
        TimerManager::Get().Update();
    }


    void Application::Update()
    {
        // シーンの遷移の進行を見て、フェードを始めたり終わらせたりするので、Fadeの更新より前に呼ぶ。
        SceneManager::Get().Update();
        Fade::Get().Update();

        m_soundDebugUI->Draw();
        m_paramDebugUI->Draw();
        UpdateCollisionDraw();
    }


    void Application::Render(RenderContext& rc)
    {
        SceneManager::Get().Render(rc);
    }


    void Application::RenderUI(RenderContext& rc)
    {
#ifdef K2_DEBUG
        // 当たり判定の線を描く。ポストプロセスがフレームバッファーを上書きした後でないと見えないので、ここで描く。
        // NOTE: 3Dの深度はG-Bufferにあり、フレームバッファーの深度は空なので、線は壁やモデルの奥でも手前に見える。
        //       UIやフェードの下に敷くため、それらより先に描く。有効にしていない時や、_DEBUGでない時は、エンジン側で何もしない。
        g_engine->DebubDrawWorld();
#endif // K2_DEBUG

        // シーンのUIを描いてから、フェードを描く。フェードは最前面に出したいので、一番最後。
        SceneManager::Get().RenderUI(rc);
        Fade::Get().Render(rc);
    }


    void Application::UpdateCollisionDraw()
    {
#ifdef BALLOON_IMGUI_ENABLED
        // F4キーで表示・非表示を切り替える。(F2はサウンド、F3はパラメーター)
        if (ImGui::IsKeyPressed(ImGuiKey_F4, false))
        {
            m_isDrawCollision = !m_isDrawCollision;

            PhysicsWorld* physicsWorld = PhysicsWorld::GetInstance();
            if (m_isDrawCollision)
            {
                physicsWorld->EnableDrawDebugWireFrame();
            }
            else
            {
                physicsWorld->DisableDrawDebugWireFrame();
            }
        }
#endif // BALLOON_IMGUI_ENABLED
    }
} // namespace app
