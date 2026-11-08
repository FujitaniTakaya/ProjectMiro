/**
 * @file BlurredBackdrop.cpp
 * @brief 前の画面をぼかした背景
 */
#include "stdafx.h"

#include "BlurredBackdrop.h"


namespace app
{
    namespace ui
    {
        BlurredBackdrop::BlurredBackdrop()
            : m_captureTarget()
            , m_blur()
            , m_sprite()
            , m_isCaptured(false)
        {
            // NOTE: 描画先の形式は、フレームバッファ(R8G8B8A8_UNORM + D32_FLOAT)に合わせる。
            //       UIの描画(スプライト・フォント)のパイプラインが、この形式の描画先を前提に作られているため。
            m_captureTarget.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);

            // 取り込み先をぼかす。ポーズ画面(RenderingEngine のシーンぼかし)と同じ DualBlur なので、同じぼけ方になる。
            m_blur.Init(&m_captureTarget.GetRenderTargetTexture());

            // ぼかした画像を、全画面に、不透明で描くスプライト。
            // NOTE: m_blur.GetResultTexture() を参照するので、必ず m_blur.Init の後に Init する。
            SpriteInitData initData;
            initData.m_width = FRAME_BUFFER_W;
            initData.m_height = FRAME_BUFFER_H;
            initData.m_fxFilePath = SpriteRender::DEFAULT_FX_PATH;
            initData.m_textures[0] = &m_blur.GetResultTexture();
            m_sprite.Init(initData);
        }


        BlurredBackdrop::~BlurredBackdrop() = default;


        void BlurredBackdrop::Capture(RenderContext& rc, const std::function<void(RenderContext&)>& drawScreen)
        {
            // 前の画面を、取り込み先に描く。
            rc.WaitUntilToPossibleSetRenderTarget(m_captureTarget);
            rc.SetRenderTargetAndViewport(m_captureTarget);
            rc.ClearRenderTargetView(m_captureTarget);
            drawScreen(rc);
            rc.WaitUntilFinishDrawingToRenderTarget(m_captureTarget);

            // ぼかす。
            m_blur.ExecuteOnGPU(rc);

            // 描画先をフレームバッファに戻す。(RenderingEngine::ExecutePostProcess() と同じ手順)
            g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
            rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

            m_isCaptured = true;
        }


        void BlurredBackdrop::Render(RenderContext& rc)
        {
            if (!m_isCaptured)
            {
                return;
            }

            m_sprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_sprite.Draw(rc);
        }
    } // namespace ui
} // namespace app
