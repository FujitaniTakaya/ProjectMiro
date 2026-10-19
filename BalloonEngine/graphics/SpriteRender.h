/**
 * @file SpriteRender.h
 * @brief スプライト描画クラスの宣言
 */
#pragma once
#include "IRenderObject.h"
#include "math/Transform.h"


namespace nsBalloonEngine
{
    /**
     * @brief スプライト描画クラス
     * @details 画像1枚を、座標・回転・拡大・基点・乗算色を指定して描画する。アルファブレンドのモードも指定できる。
     *          描画は即時なので、UIなどポストプロセスの後に描きたいものは、RenderingEngine::Execute() の後(Game側のUIの描画)で
     *          Draw() を呼ぶこと。それより前に呼ぶと、ポストプロセスに上書きされて見えなくなる。
     */
    class SpriteRender : public IRenderObject
    {
    public:
        /** 既定のスプライトシェーダーのパス */
        static constexpr const char* DEFAULT_FX_PATH = "Assets/shader/balloon/sprite.fx";


    public:
        SpriteRender();
        ~SpriteRender() override;


    public:
        /**
         * @brief 初期化関数
         * @param ddsFilePath 画像アセットのファイルパス
         * @param width 幅
         * @param height 高さ
         * @param fxFilePath シェーダーアセットのファイルパス
         * @param alphaBlendMode アルファブレンドのモード。AlphaBlendMode_None は、不透明で上書きする。
         *                       乗算色のアルファで半透明にしたいときは、AlphaBlendMode_Trans を指定する。
         */
        void Init(
            const char* ddsFilePath,
            const uint32_t width,
            const uint32_t height,
            const char* fxFilePath = DEFAULT_FX_PATH,
            const AlphaBlendMode alphaBlendMode = AlphaBlendMode_None
        );


        /**
         * @brief 更新関数
         * @details 設定した座標・回転・拡大・基点・乗算色を、スプライトに反映する。Draw() の前に呼ぶこと。
         *          SetPosition() や SetMulColor() などは値を覚えるだけで、この Update() を呼ぶまで描画には反映されない。
         */
        void Update();


        /**
         * @brief 描画関数
         * @note Init() の後に呼ぶこと。
         * @param rc レンダーコンテキスト
         */
        void Draw(RenderContext& rc);


        //=======================================================================
        // トランスフォーム
        //=======================================================================
    public:
        /**
         * @brief 座標・回転・拡大を設定
         * @param position 座標
         * @param rotation 回転角度
         * @param scale 拡大率
         */
        void SetTRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        /**
         * @brief 座標・回転・拡大を設定
         * @param transform トランスフォーム
         */
        void SetTRS(const Transform& transform);
        /**
         * @brief 座標を設定
         * @param position 座標
         */
        void SetPosition(const Vector3& position);
        /**
         * @brief 回転を設定
         * @param rotation 回転角度
         */
        void SetRotation(const Quaternion& rotation);
        /**
         * @brief 拡大を設定
         * @param scale 拡大率
         */
        void SetScale(const Vector3& scale);
        /**
         * @brief トランスフォームを取得
         * @return トランスフォーム
         */
        const Transform& GetTransform() const;


        //=======================================================================
        // カラー
        //=======================================================================
    public:
        /**
         * @brief 乗算色を設定
         * @details 値を覚えるだけで、Update() を呼ぶまで描画には反映されない。(色だけを毎フレーム変えて描くときも、Update() を呼ぶこと。)
         *          アルファを使った半透明にするには、Init() で AlphaBlendMode_Trans を指定すること。
         * @param mulColor 乗算色
         */
        void SetMulColor(const Vector4& mulColor);
        /**
         * @brief 乗算色を取得
         * @return 乗算色
         */
        const Vector4& GetMulColor() const;


        //=======================================================================
        // 基点
        //=======================================================================
    public:
        /**
         * @brief 基点を設定
         * @param pivot 基点
         */
        void SetPivot(const Vector2& pivot);
        /**
         * @brief 基点を取得
         * @return 基点
         */
        const Vector2& GetPivot() const;


    private:
        /** スプライトデータ */
        Sprite m_sprite;
        /** トランスフォーム */
        Transform m_transform;
        /** 乗算色 */
        Vector4 m_mulColor;
        /** 基点 */
        Vector2 m_pivot;
    };
} // namespace nsBalloonEngine
