/**
 * @file CoreHPBarMenu.cpp
 * @brief コアの耐久値バーを動かすMenu
 */
#include "stdafx.h"

#include "CoreHPBarMenu.h"

#include <cmath>


namespace app
{
    namespace ui
    {
        CoreHPBarMenu::CoreHPBarMenu()
            : m_screenPosition(0.0f, 0.0f)
            , m_isVisible(false)
        {}


        CoreHPBarMenu::~CoreHPBarMenu() = default;


        void CoreHPBarMenu::Update()
        {
            // NOTE: キャンバスは作り直されないので一度書けば足りるが、キャンバスが後から作られる場合(jsonが後から読み込めた時)に備えて、毎フレーム書く。
            if (m_canvas)
            {
                m_canvas->m_transform.m_localTransform.m_position = Vector3(m_screenPosition.x, m_screenPosition.y, 0.0f);
            }

            HPBarMenu::Update();
        }


        void CoreHPBarMenu::Render(RenderContext& rc)
        {
            if (m_isVisible)
            {
                HPBarMenu::Render(rc);
            }
        }


        void CoreHPBarMenu::UpdateScreenPosition(const Vector3& worldPosition)
        {
            m_isVisible = false;
            if (g_camera3D == nullptr)
            {
                return;
            }

            // クリップ座標で、カメラの視錐台の中にあるかを調べる。
            // NOTE: CalcScreenPositionFromWorldPosition() は w で割るだけなので、カメラの後ろ(w <= 0)の点は、画面の反対側の位置になってしまう。
            //       画面の外か、後ろかは、その結果からは分からないので、ここで先に判定する。深度は、DirectXの 0 <= z <= w。
            //       (w < 0 は |x| > w でも弾かれるが、後ろを弾く意図を示すために w <= 0 を先頭に置いている。)
            Vector4 clip(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
            g_camera3D->GetViewProjectionMatrix().Apply(clip);
            if (clip.w <= 0.0f || std::fabs(clip.x) > clip.w || std::fabs(clip.y) > clip.w || clip.z < 0.0f || clip.z > clip.w)
            {
                return;
            }

            g_camera3D->CalcScreenPositionFromWorldPosition(m_screenPosition, worldPosition);
            m_isVisible = true;
        }
    } // namespace ui
} // namespace app
