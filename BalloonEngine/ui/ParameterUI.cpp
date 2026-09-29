/**
 * @file ParameterUI.cpp
 * @brief エンジンのパラメータ調整用UI(ImGui)クラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "ParameterUI.h"

#include "imgui.h"


namespace nsBalloonEngine
{
    void ParameterUI::Draw()
    {
        // F1キーで表示・非表示を切り替える。
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false))
        {
            m_isVisible = !m_isVisible;
        }
        if (!m_isVisible)
        {
            return;
        }

        // NOTE: Game側が同名のウィンドウを作ると、1つのウィンドウに混ざってしまう。
        //       Game側は "BalloonEngine" 以外の名前を使うこと。
        if (ImGui::Begin("BalloonEngine", &m_isVisible))
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::TextDisabled("F1: show / hide");

            DrawDirectionLight();
            DrawAmbientLight();
            DrawPointLight();
            DrawSpotLight();
            DrawBloom();
            DrawDepthOfField();
            DrawDebug();
        }
        ImGui::End();
    }


    void ParameterUI::DrawDirectionLight()
    {
        if (!ImGui::CollapsingHeader("Direction Light"))
        {
            return;
        }

        auto& light = SceneLight::Get().m_sceneLight;

        ImGui::SliderInt("DirectionLightNum", &light.usingDirectionLightNum, 0, LightingCB::MAX_DIRECTION_LIGHT_NUM);
        for (int i = 0; i < light.usingDirectionLightNum; ++i)
        {
            ImGui::PushID(i);
            if (ImGui::TreeNode("", "Light %d", i))
            {
                auto& it = light.directionLights.at(i);
                ImGui::SliderFloat3("Direction", &it.lightDir.x, -1.0f, 1.0f);
                ImGui::ColorEdit3("Color", &it.lightColor.m_colorVec3.x);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::SliderFloat("Shininess", &light.shininess, 1.0f, 200.0f);
        ImGui::SliderFloat("Spec Intensity", &light.specIntensity, 0.0f, 5.0f);
        ImGui::SliderFloat("Bias", &light.localBias, 0.000001f, 1.0f);
    }


    void ParameterUI::DrawAmbientLight()
    {
        if (!ImGui::CollapsingHeader("Ambient Light"))
        {
            return;
        }

        auto& light = SceneLight::Get().m_sceneLight;
        ImGui::ColorEdit3("Ambient", &light.ambientLight.lightColor.m_colorVec3.x);
    }


    void ParameterUI::DrawPointLight()
    {
        if (!ImGui::CollapsingHeader("Point Lights"))
        {
            return;
        }

        auto& light = SceneLight::Get().m_sceneLight;

        ImGui::SliderInt("PointLightNum", &light.usingPointLightNum, 0, LightingCB::MAX_POINT_LIGHT_NUM);
        for (int i = 0; i < light.usingPointLightNum; ++i)
        {
            ImGui::PushID(i);
            if (ImGui::TreeNode("", "Light %d", i))
            {
                auto& it = light.pointLights.at(i);
                ImGui::SliderFloat3("Position", &it.position.x, -1000.0f, 1000.0f);
                ImGui::DragFloat("Range", &it.range, 5.0f, 10.0f, 2000.0f);
                ImGui::ColorEdit3("Color", &it.lightColor.m_colorVec3.x);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }


    void ParameterUI::DrawSpotLight()
    {
        if (!ImGui::CollapsingHeader("Spot Lights"))
        {
            return;
        }

        auto& light = SceneLight::Get().m_sceneLight;

        ImGui::SliderInt("SpotLightNum", &light.usingSpotLightNum, 0, LightingCB::MAX_SPOT_LIGHT_NUM);
        for (int i = 0; i < light.usingSpotLightNum; ++i)
        {
            ImGui::PushID(i);
            if (ImGui::TreeNode("", "Light %d", i))
            {
                auto& it = light.spotLights.at(i);
                ImGui::SliderFloat3("Position", &it.pointLight.position.x, -1000.0f, 1000.0f);
                ImGui::DragFloat("Range", &it.pointLight.range, 5.0f, 10.0f, 2000.0f);
                ImGui::ColorEdit3("Color", &it.pointLight.lightColor.m_colorVec3.x);
                ImGui::SliderFloat3("Direction", &it.lightDir.x, -1.0f, 1.0f);
                ImGui::SliderFloat("Angle", &it.angle, Math::DegToRad(1.0f), Math::DegToRad(90.0f));
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }


    void ParameterUI::DrawBloom()
    {
        if (!ImGui::CollapsingHeader("Bloom"))
        {
            return;
        }

        auto& re = RenderingEngine::Get();

        auto& bloomCB = re.GetBloomCB();
        ImGui::SliderFloat("BloomThreshold", &bloomCB.threshold, 1.0f, 3.0f);
        ImGui::Checkbox("Dual Blur", &re.SetDualBlurEnable());
        ImGui::SliderFloat("BloomIntensity", &re.GetBloomIntensity(), 0.0f, 3.0f);
    }


    void ParameterUI::DrawDepthOfField()
    {
        if (!ImGui::CollapsingHeader("Depth of Field"))
        {
            return;
        }

        auto& re = RenderingEngine::Get();

        auto& dofCB = re.GetDoFCB();
        ImGui::Checkbox("Enable DoF", &re.GetDoFEnable());
        ImGui::SliderFloat("Focus Distance", &dofCB.focusDistance, 0.0f, 3000.0f);
        ImGui::SliderFloat("Focus Range", &dofCB.focusRange, 10.0f, 2000.0f);
    }


    void ParameterUI::DrawDebug()
    {
        if (!ImGui::CollapsingHeader("Debug"))
        {
            return;
        }

        auto& re = RenderingEngine::Get();
        ImGui::Checkbox("Draw G-Buffer", &re.GetDebugDrawGBufferEnable());
    }
} // namespace nsBalloonEngine
