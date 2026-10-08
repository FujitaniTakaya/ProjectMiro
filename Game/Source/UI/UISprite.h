/**
 * @file UISprite.h
 * @brief UIの画像1枚を描画するクラス
 * @details k2EngineLowのSpriteを包んで、座標・回転・拡大・基点・乗算色を設定できるようにしたもの。
 *          BalloonEngineのSpriteRenderと違い、アルファブレンドを指定できる。(既定は半透明)
 *          描画は即時なので、RenderUI()の中でDraw()を呼ぶこと。Render()の中だと、ポストプロセスに上書きされて見えなくなる。
 */
#pragma once


namespace app
{
    namespace ui
    {
        /**
         * @brief UIの画像1枚を描画するクラス
         */
        class UISprite : public Noncopyable
        {
        public:
            UISprite();
            ~UISprite();


        public:
            /**
             * @brief 初期化
             * @param ddsFilePath 画像(DDS)のファイルパス
             * @param width 幅
             * @param height 高さ
             * @param alphaBlendMode アルファブレンドのモード
             */
            void Init(
                const char* ddsFilePath,
                const uint32_t width,
                const uint32_t height,
                const AlphaBlendMode alphaBlendMode = AlphaBlendMode_Trans
            );


            /**
             * @brief 更新
             * @details 設定した座標・回転・拡大・基点を、スプライトに反映する。Draw()の前に呼ぶこと。
             */
            void Update();


            /**
             * @brief 描画
             * @note Init()の後に呼ぶこと。
             * @param rc レンダーコンテキスト
             */
            void Draw(RenderContext& rc);


        public:
            /**
             * @brief 座標を設定
             * @param position 座標
             */
            void SetPosition(const Vector3& position);

            /**
             * @brief 回転を設定
             * @param rotation 回転
             */
            void SetRotation(const Quaternion& rotation);

            /**
             * @brief 拡大を設定
             * @param scale 拡大率
             */
            void SetScale(const Vector3& scale);

            /**
             * @brief 基点を設定
             * @details (0, 0)が左下、(1, 1)が右上。
             * @param pivot 基点
             */
            void SetPivot(const Vector2& pivot);

            /**
             * @brief 乗算色を設定
             * @param mulColor 乗算色
             */
            void SetMulColor(const Vector4& mulColor);


        private:
            /** スプライト */
            Sprite m_sprite;
            /** 座標・回転・拡大 */
            Transform m_transform;
            /** 基点 */
            Vector2 m_pivot;
        };
    } // namespace ui
} // namespace app
