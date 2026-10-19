/**
 * @file SDFFontEngine.h
 * @brief SDFフォント描画エンジンの宣言
 * @details msdf-atlas-gen が出力したアトラス画像(PNG)とグリフ情報(JSON)を読み込んで、文字を描画する。
 *          フォントは複数登録できる。LoadFont() が返す FontId で、描画するフォントを選ぶ。
 *          SpriteBatch + 独自のピクセルシェーダー(shader/sdfFont.fx)で描くので、拡大しても輪郭が鮮明。
 *          RenderingEngine が Initialize() / Finalize() を呼ぶ。Game側は LoadFont() と、FontRender を使う。
 *
 *          アトラスは msdf-atlas-gen で作る。例:
 *          msdf-atlas-gen.exe -font xxx.ttf -type sdf -format png -charset charset.txt -imageout xxx.png -json xxx.json -size 48
 */
#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>


namespace nsBalloonEngine
{
    /** フォントのID。SDFFontEngine::LoadFont() が返す。 */
    using FontId = uint32_t;

    /** 無効なフォントID */
    inline constexpr FontId INVALID_FONT_ID = UINT32_MAX;


    /**
     * @brief テキストの水平アライメント
     */
    enum class TextAlign
    {
        Left,
        Center,
        Right,
    };


    /**
     * @brief msdf-atlas-gen が出力する1グリフ分のレイアウト情報
     */
    struct SDFGlyph
    {
        /** 次の文字までの送り幅(emスケール) */
        float advance;
        // planeBounds: ベースライン原点・emスケールでのクワッド範囲(Y上が正)
        float planeLeft, planeBottom, planeRight, planeTop;
        // atlasBounds: アトラステクスチャ上のピクセル座標(yOrigin:bottom 前提)
        float atlasLeft, atlasBottom, atlasRight, atlasTop;
    };


    /**
     * @brief SDFフォント描画エンジン
     */
    class SDFFontEngine : public Noncopyable
    {
    private:
        SDFFontEngine();
        ~SDFFontEngine();


    public:
        /** 登録できるフォントの数の上限。SRV用ディスクリプタヒープの大きさになる。 */
        static constexpr uint32_t MAX_FONT_NUM = 8;


    public:
        /**
         * @brief 初期化
         * @details SDFシェーダーのコンパイル、SpriteBatchの作成、SRV用ディスクリプタヒープの作成を行う。
         *          RenderingEngine::Initialize() から呼ばれる。
         */
        void Initialize();

        /**
         * @brief 終了
         * @details 登録したフォントも全て破棄する。グラフィックスエンジンを破棄する前に呼ぶこと。
         */
        void Finalize();

        /**
         * @brief フォントを登録する
         * @details アトラス画像とグリフ情報を読み込む。Initialize() の後に呼ぶこと。
         *          失敗した場合や、MAX_FONT_NUM を超える場合は、ログを出して INVALID_FONT_ID を返す。
         * @param atlasPngPath アトラス画像(PNG)のファイルパス
         * @param atlasJsonPath グリフ情報(JSON、msdf-atlas-gen の出力)のファイルパス
         * @return フォントID。失敗した場合は INVALID_FONT_ID
         */
        FontId LoadFont(const char* atlasPngPath, const char* atlasJsonPath);

        /**
         * @brief 登録済みのフォントかどうか
         * @param fontId フォントID
         * @return 登録済みならtrue
         */
        bool IsValidFont(const FontId fontId) const
        {
            return fontId < m_fonts.size();
        }

        /**
         * @brief フォントのem(拡大率1のときの文字の高さ。ピクセル)を取得
         * @param fontId フォントID
         * @return em(ピクセル)。無効なフォントIDなら0
         */
        float GetEmSize(const FontId fontId) const;


    public:
        /**
         * @brief 描画開始
         * @details SpriteBatch::Begin を呼ぶ。
         * @param rc レンダーコンテキスト
         * @param rotation Z軸回転(ラジアン)。0で無回転。
         * @param gameSpacePos 回転の中心となるゲーム座標。FontRender の位置を渡す。
         */
        void BeginDraw(RenderContext& rc, float rotation = 0.0f, Vector2 gameSpacePos = Vector2(0.0f, 0.0f));

        /**
         * @brief テキストを描画する
         * @details BeginDraw() と EndDraw() の間で呼ぶ。影・アライメント・行間は、SetShadowParam() などで設定した値が使われる。
         *          回転は BeginDraw() で掛けるので、ここには渡さない。
         * @param fontId フォントID。無効な場合は何も描画しない。
         * @param text 描画するワイド文字列。'\n' で改行する。
         * @param position 画面の中心を原点にした座標(y上が正)
         * @param color RGBAカラー
         * @param scale 拡大率。1でフォントのem(アトラス生成時の -size)ぶんの高さ
         * @param pivot 基点。(0, 0)=左下、(0.5, 0.5)=中央、(1, 1)=右上
         */
        void Draw(
            const FontId fontId,
            const wchar_t* text,
            const Vector2& position,
            const Vector4& color,
            const Vector2& scale,
            Vector2 pivot
        );

        /**
         * @brief 描画終了
         * @details SpriteBatch::End を呼ぶ。
         * @param rc レンダーコンテキスト
         */
        void EndDraw(RenderContext& rc);


    public:
        /**
         * @brief 影のパラメーターを設定(Draw の直前に呼ぶ)
         * @param enable 影を描画するか
         * @param offset 影のオフセット(ピクセル)
         * @param color 影の色
         */
        void SetShadowParam(bool enable, float offset, const Vector4& color)
        {
            m_isDrawShadow = enable;
            m_shadowOffset = offset;
            m_shadowColor = color;
        }

        /**
         * @brief 水平アライメントを設定(Draw の直前に呼ぶ)
         * @param align Left / Center / Right
         */
        void SetTextAlign(TextAlign align)
        {
            m_textAlign = align;
        }

        /**
         * @brief 行間倍率を設定(Draw の直前に呼ぶ)
         * @param lineSpacing 1.0=行の高さぴったり、1.2=1.2倍
         */
        void SetLineSpacing(float lineSpacing)
        {
            m_lineSpacing = lineSpacing;
        }


    private:
        /**
         * @brief フォント1つぶんのデータ
         */
        struct FontData
        {
            /** アトラステクスチャ */
            ComPtr<ID3D12Resource> m_texture;
            /** SRVのGPUハンドル */
            D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
            /** アトラスの幅(ピクセル) */
            UINT m_atlasWidth;
            /** アトラスの高さ(ピクセル) */
            UINT m_atlasHeight;
            /** emスケール(ピクセル) */
            float m_emSize;
            /** 行の高さ(emスケール) */
            float m_lineHeight;
            /** ベースラインから上端までの距離(emスケール) */
            float m_ascender;
            /** Unicodeコードポイントをキーとするグリフデータのマップ */
            std::unordered_map<uint32_t, SDFGlyph> m_glyphs;
        };


    private:
        /**
         * @brief グリフ情報(JSON)を読み込む
         * @param jsonPath JSONのファイルパス
         * @param font 読み込み先
         * @return 読み込めたらtrue
         */
        bool LoadGlyphs(const char* jsonPath, FontData& font) const;

        /**
         * @brief 実際に SpriteBatch::Draw を呼ぶ内部実装
         * @param font 使うフォント
         * @param text 描画するワイド文字列
         * @param screenPos 画面(左上原点)の座標
         * @param color 色
         * @param scale 拡大率
         * @param pivot 基点
         */
        void DrawString(
            const FontData& font,
            const wchar_t* text,
            const Vector2& screenPos,
            const Vector4& color,
            const Vector2& scale,
            const Vector2& pivot
        );


    private:
        /** SDFシェーダーのパス */
        static constexpr const char* SHADER_PATH = "Assets/shader/balloon/sdfFont.fx";


    private:
        /** SpriteBatch。全てのフォントで共有する。 */
        std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
        /** SDFのピクセルシェーダー */
        Shader m_psShader;
        /** 全てのフォントのSRVを置くディスクリプタヒープ */
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        /** ディスクリプタ1つぶんのサイズ */
        UINT m_srvDescriptorSize;
        /** 登録されたフォント。FontIdが添え字 */
        std::vector<FontData> m_fonts;

        /** 影を描画するかどうか */
        bool m_isDrawShadow;
        /** 影のオフセット(ピクセル) */
        float m_shadowOffset;
        /** 影のカラー */
        Vector4 m_shadowColor;
        /** 水平アライメント */
        TextAlign m_textAlign;
        /** 行間倍率 */
        float m_lineSpacing;


    public:
        /**
         * @brief インスタンスを取得
         * @return インスタンス
         */
        static SDFFontEngine& Get()
        {
            static SDFFontEngine instance;
            return instance;
        }
    };
} // namespace nsBalloonEngine
