#include "stdafx.h"
#include "system/system.h"

#include<InitGUID.h>
#include<dxgidebug.h>

#include "Application.h"



void ReportLiveObjects()
{
	IDXGIDebug* pDxgiDebug;

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDxgiDebug);

	// 出力。
	pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
}

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数。
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//ゲームの初期化。
	InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
	//////////////////////////////////////
	// ここから初期化を行うコードを記述する。
	//////////////////////////////////////

	RenderingEngine::Get().Initialize();

	// アプリケーションを作成。NewGOは使わず、アプリ側の処理はApplicationが持つ。
	// NOTE: RenderingEngineの初期化後に作ること。
	auto application = std::make_unique<app::Application>();

	//////////////////////////////////////
	// 初期化を行うコードを書くのはここまで！！！
	//////////////////////////////////////
	
	// ここからゲームループ。
	while (DispatchWindowMessage())
	{
		if (g_pad[0]->IsTrigger(enButtonA) ){
			g_pad[0]->SetVibration(/*durationSec=*/0.5f, /*normalizedPower=*/1.0f);
		}
		g_engine->BeginFrame();		// フレームの開始。
		application->PreUpdate();	// ExecuteUpdateの前の更新。(サウンド・エフェクトの回収。必ず前に呼ぶ。)
		g_engine->ExecuteUpdate();	// パッド・サウンド・エフェクトの更新。
		application->Update();		// アプリ側の更新。
		g_engine->ExecuteRender();	// NewGOしたオブジェクト(サウンド・エフェクト)の描画。
		application->Render(g_graphicsEngine->GetRenderContext());	// アプリ側の描画。
		RenderingEngine::Get().Execute();	// BalloonEngineの描画。
		application->RenderUI(g_graphicsEngine->GetRenderContext());	// アプリ側のUIの描画。(ポストプロセスの後に描くので、画面を上書きされない。)
		g_engine->EndFrame();		// フレームの終了。
	}

	application.reset();	// アプリケーションを破棄。エンジンを破棄する前に行う。

	RenderingEngine::Get().Finalize();	// ImGuiなどの終了処理。エンジンを破棄する前に呼ぶ。
	FinalizeGame();

#ifdef _DEBUG
	ReportLiveObjects();
#endif // _DEBUG
	return 0;
}

