/**
 * @file Application.cpp
 * @brief アプリケーション全体を管理するクラス
 */
#include "stdafx.h"

#include "Application.h"
#include "Game.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Parameter/HotReloadManager.h"
#include "Source/Parameter/ParamDebugUI.h"
#include "Source/Parameter/ParamHolder.h"
#include "Source/Scene/Fade.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundDebugUI.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Timer/TimerManager.h"


namespace app
{
	Application::Application()
	{
		SoundManager::CreateInstance();
		EffectManager::CreateInstance();
		ParamHolder::CreateInstance();
		Fade::CreateInstance();
		SceneManager::CreateInstance();

		m_game = std::make_unique<Game>();
		m_game->StartWrapper();

		m_soundDebugUI = std::make_unique<SoundDebugUI>();
		m_paramDebugUI = std::make_unique<ParamDebugUI>();
	}


	Application::~Application()
	{
		m_paramDebugUI.reset();
		m_soundDebugUI.reset();
		m_game.reset();

		SceneManager::DestroyInstance();
		Fade::DestroyInstance();
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
		m_game->UpdateWrapper();

		// シーンの遷移の進行を見て、フェードを始めたり終わらせたりするので、Fadeの更新より前に呼ぶ。
		SceneManager::Get().Update();
		Fade::Get().Update();

		m_soundDebugUI->Draw();
		m_paramDebugUI->Draw();
	}


	void Application::Render(RenderContext& rc)
	{
		m_game->RenderWrapper(rc);
		SceneManager::Get().Render(rc);
	}


	void Application::RenderUI(RenderContext& rc)
	{
		// フェードは最前面に出したいので、一番最後に描く。
		Fade::Get().Render(rc);
	}
}
