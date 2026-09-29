#include "stdafx.h"
#include "Game.h"


bool Game::Start()
{
	m_modelRender.Init("Assets/modelData/unityChan.tkm", nullptr, 0, EnModelUpAxis::enModelUpAxisY, true, true);
	Quaternion rot;
	rot.SetRotationDegY(180.0f);
	m_modelRender.SetRotation(rot);

	m_bgModelRender.Init("Assets/modelData/ground.tkm", nullptr, 0, EnModelUpAxis::enModelUpAxisZ, true, false);
	m_bgModelRender.Update();

	g_camera3D->SetNear(1.0f);
	g_camera3D->SetFar(10000.0f);

	SceneLight::Get().m_sceneLight.ambientLight.lightColor = { 0.5f, 0.5f, 0.5f };

	return true;
}

void Game::Update()
{
#ifdef BALLOON_IMGUI_ENABLED
	ImGui::Begin("Parameter");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();
#endif // BALLOON_IMGUI_ENABLED

	// g_renderingEngine->DisableRaytracing();
	m_modelRender.Update();
	// カメラ(BalloonEngineのGameCameraと同じ値。)
	Vector3 target = m_modelRender.GetTransform().m_position;
	target.y += 80.0f;
	g_camera3D->SetTarget(target);
	g_camera3D->SetPosition(target + Vector3(0.0f, 125.0f, -250.0f));
	g_camera3D->Update();
}

void Game::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
	m_bgModelRender.Draw(rc);
}