/**
 * @file BlurredBackdrop.h
 * @brief 前の画面をぼかした背景
 * @details 前の画面をオフスクリーンに描いて(取り込み)、ぼかして、全画面の背景として描く。
 *          取り込みとぼかしは、Capture() を呼んだ時の1回だけ。毎フレームの描画は、全画面スプライト1枚。
 *          エンジンのシーンぼかし(RenderingEngine::SetSceneBlurRate。ポーズ画面が使う)は3Dのメインレンダーターゲットだけが対象で、
 *          ポストプロセスの後に描くUIは映らないので、UIを背景にしたい時は、このクラスで取り込む。
 *          ぼかしは、シーンぼかしと同じ DualBlur なので、ポーズ画面と同じぼけ方になる。
 *          例: backdrop.Capture(rc, [&](RenderContext& context) { packet->Render(context); });
 *              backdrop.Render(rc);
 */
#pragma once
#include <functional>


namespace app
{
    namespace ui
    {
        /**
         * @brief 前の画面をぼかした背景
         */
        class BlurredBackdrop : public Noncopyable
        {
        public:
            BlurredBackdrop();
            ~BlurredBackdrop();


        public:
            /**
             * @brief 前の画面を取り込んで、ぼかす
             * @details 取り込み先のレンダーターゲットに、drawScreen で画面を描いて、ぼかす。終わったら、描画先をフレームバッファに戻す。
             *          RenderUI() の中(フレームバッファに描いている最中)で呼ぶこと。呼ぶ度に、前の取り込みを置き換える。
             *          取り込み先は、フレームバッファと同じ大きさ・形式なので、drawScreen には、フレームバッファに描く時と同じ描画を書く。
             * @param rc レンダーコンテキスト
             * @param drawScreen 取り込む画面を描く関数
             */
            void Capture(RenderContext& rc, const std::function<void(RenderContext&)>& drawScreen);

            /**
             * @brief ぼかした画面を、全画面に描く
             * @details Capture() を呼ぶ前は、何も描かない。
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc);

            /**
             * @brief 取り込み済みかどうか
             * @return Capture() を呼んでいればtrue
             */
            bool IsCaptured() const
            {
                return m_isCaptured;
            }


        private:
            /** 取り込み先のレンダーターゲット。フレームバッファと同じ大きさ・形式 */
            RenderTarget m_captureTarget;
            /** デュアルブラー。取り込み先を、縮小・拡大を重ねてぼかす */
            DualBlur m_blur;
            /** ぼかした画像を全画面に描くスプライト */
            Sprite m_sprite;
            /** 取り込み済みかどうか */
            bool m_isCaptured;
        };
    } // namespace ui
} // namespace app
