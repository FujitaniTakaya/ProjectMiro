/**
 * @file Application.cpp
 * @brief アプリケーション全体を管理するクラス
 */
#include "stdafx.h"

#include "Application.h"
#include "Source/Effect/EffectManager.h"
#include "Game.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	Application::Application()
	{
		// NOTE: Gameが Start() の中でマネージャーを使えるよう、マネージャーを先に作る。
		SoundManager::CreateInstance();
		EffectManager::CreateInstance();

		m_game = std::make_unique<Game>();
		m_game->StartWrapper();
	}


	Application::~Application()
	{
		// 作った順の逆に破棄する。(Gameの破棄中にマネージャーを使えるように)
		m_game.reset();
		EffectManager::DestroyInstance();
		SoundManager::DestroyInstance();
	}


	void Application::PreUpdate()
	{
		SoundManager::Get().Update();
		EffectManager::Get().CollectFinished();
	}


	void Application::Update()
	{
		m_game->UpdateWrapper();
		EffectManager::Get().UpdateFollow();
	}


	void Application::Render(RenderContext& rc)
	{
		m_game->RenderWrapper(rc);
	}
}
