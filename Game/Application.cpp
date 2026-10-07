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
#include "Source/Sound/SoundDebugUI.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	Application::Application()
	{
		SoundManager::CreateInstance();
		EffectManager::CreateInstance();
		ParamHolder::CreateInstance();

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

		ParamHolder::DestroyInstance();
		EffectManager::DestroyInstance();
		SoundManager::DestroyInstance();
	}


	void Application::PreUpdate()
	{
		HotReloadManager::Get().Update();
		SoundManager::Get().Update();
		EffectManager::Get().Update();
	}


	void Application::Update()
	{
		m_game->UpdateWrapper();

		m_soundDebugUI->Draw();
		m_paramDebugUI->Draw();
	}


	void Application::Render(RenderContext& rc)
	{
		m_game->RenderWrapper(rc);
	}
}
