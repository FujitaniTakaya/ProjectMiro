/**
 * @file SDFFontEngine.cpp
 * @brief SDFフォント描画エンジンの実装
 */
#include "BalloonEnginePreCompile.h"

#include "SDFFontEngine.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

#include "DirectXTK/Inc/ResourceUploadBatch.h"
#include "DirectXTK/Inc/WICTextureLoader.h"


namespace nsBalloonEngine
{
    using namespace DirectX;

    namespace
    {
        /** 1つのテキストの最大行数 */
        constexpr int MAX_LINES = 32;


        /**
         * @brief UTF-8の文字列をワイド文字列に変換する
         */
        std::wstring ToWide(const char* text)
        {
            const int length = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
            if (length <= 1)
            {
                return std::wstring();
            }
            std::wstring wide(static_cast<size_t>(length), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, text, -1, wide.data(), length);
            wide.resize(static_cast<size_t>(length) - 1);
            return wide;
        }


        // 以下の2つは msdf-atlas-gen の出力専用のJSONパーサー。

        /** "key": <float> を fromPos 以降で最初に見つけて返す */
        float JsonFloat(const std::string& json, size_t fromPos, const char* key)
        {
            const std::string searchKey = std::string("\"") + key + "\"";
            const size_t pos = json.find(searchKey, fromPos);
            if (pos == std::string::npos)
            {
                return 0.0f;
            }
            const size_t colonPos = json.find(':', pos + searchKey.size());
            if (colonPos == std::string::npos)
            {
                return 0.0f;
            }
            float v = 0.0f;
            sscanf_s(json.c_str() + colonPos + 1, " %f", &v);
            return v;
        }


        /** "key": <int> を fromPos 以降で最初に見つけて返す */
        int JsonInt(const std::string& json, size_t fromPos, const char* key)
        {
            const std::string searchKey = std::string("\"") + key + "\"";
            const size_t pos = json.find(searchKey, fromPos);
            if (pos == std::string::npos)
            {
                return 0;
            }
            const size_t colonPos = json.find(':', pos + searchKey.size());
            if (colonPos == std::string::npos)
            {
                return 0;
            }
            int v = 0;
            sscanf_s(json.c_str() + colonPos + 1, " %d", &v);
            return v;
        }
    } // namespace


    SDFFontEngine::SDFFontEngine()
        : m_srvDescriptorSize(0)
        , m_isDrawShadow(false)
        , m_shadowOffset(0.0f)
        , m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
        , m_textAlign(TextAlign::Left)
        , m_lineSpacing(1.2f)
    {}


    SDFFontEngine::~SDFFontEngine()
    {}


    //=======================================================================
    // 初期化・終了
    //=======================================================================
    void SDFFontEngine::Initialize()
    {
        auto* d3dDevice = g_graphicsEngine->GetD3DDevice();

        // SDFピクセルシェーダーをコンパイル
        m_psShader.LoadPS(SHADER_PATH, "PSMain");

        // 全てのフォントのSRVを置くディスクリプタヒープ
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.NumDescriptors = MAX_FONT_NUM;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_srvHeap.ReleaseAndGetAddressOf()))))
        {
            OutputDebugStringA("SDFFontEngine: ディスクリプタヒープの作成に失敗しました。\n");
            return;
        }
        m_srvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // SpriteBatchをカスタムSDFシェーダーで作る
        ResourceUploadBatch uploadBatch(d3dDevice);
        uploadBatch.Begin();

        RenderTargetState rtState;
        rtState.rtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtState.numRenderTargets = 1;
        rtState.dsvFormat = DXGI_FORMAT_D32_FLOAT;
        rtState.sampleMask = UINT_MAX;
        rtState.sampleDesc.Count = 1;

        D3D12_BLEND_DESC blendDesc = CommonStates::NonPremultiplied;
        SpriteBatchPipelineStateDescription psoDesc(rtState, &blendDesc);
        psoDesc.customPixelShader = {
            m_psShader.GetCompiledBlob()->GetBufferPointer(),
            m_psShader.GetCompiledBlob()->GetBufferSize()
        };

        D3D12_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(UI_SPACE_WIDTH);
        viewport.Height = static_cast<float>(UI_SPACE_HEIGHT);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        m_spriteBatch = std::make_unique<SpriteBatch>(d3dDevice, uploadBatch, psoDesc, &viewport);

        auto uploadFuture = uploadBatch.End(g_graphicsEngine->GetCommandQueue());
        uploadFuture.wait();

        m_fonts.reserve(MAX_FONT_NUM);
    }


    void SDFFontEngine::Finalize()
    {
        // GPUがフォントのリソースを使い終わってから解放する。
        // (GraphicsEngine::WaitDraw() は非公開のため、フェンスで待つ。)
        if (m_spriteBatch)
        {
            auto* device = g_graphicsEngine->GetD3DDevice();
            auto* queue = g_graphicsEngine->GetCommandQueue();
            ComPtr<ID3D12Fence> fence;
            if (SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
            {
                HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                if (event != nullptr)
                {
                    queue->Signal(fence.Get(), 1);
                    fence->SetEventOnCompletion(1, event);
                    WaitForSingleObject(event, INFINITE);
                    CloseHandle(event);
                }
            }
        }

        m_fonts.clear();
        m_spriteBatch.reset();
        m_srvHeap.Reset();
    }


    //=======================================================================
    // フォントの登録
    //=======================================================================
    bool SDFFontEngine::LoadGlyphs(const char* jsonPath, FontData& font) const
    {
        std::ifstream file(jsonPath);
        if (!file.is_open())
        {
            OutputDebugStringA("SDFFontEngine: グリフ情報(JSON)が見つかりません。\n");
            OutputDebugStringA(jsonPath);
            OutputDebugStringA("\n");
            return false;
        }
        const std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // --- atlas セクション ---
        const size_t atlasPos = json.find("\"atlas\"");
        if (atlasPos != std::string::npos)
        {
            font.m_emSize = JsonFloat(json, atlasPos, "size");
            font.m_atlasWidth = static_cast<UINT>(JsonInt(json, atlasPos, "width"));
            font.m_atlasHeight = static_cast<UINT>(JsonInt(json, atlasPos, "height"));
        }

        // --- metrics セクション ---
        const size_t metricsPos = json.find("\"metrics\"");
        if (metricsPos != std::string::npos)
        {
            font.m_lineHeight = JsonFloat(json, metricsPos, "lineHeight");
            font.m_ascender = JsonFloat(json, metricsPos, "ascender");
        }

        // --- glyphs 配列 ---
        const size_t glyphsPos = json.find("\"glyphs\"");
        if (glyphsPos == std::string::npos)
        {
            OutputDebugStringA("SDFFontEngine: JSONに\"glyphs\"がありません。\n");
            return false;
        }

        const size_t arrayStart = json.find('[', glyphsPos);
        size_t arrayEnd = json.rfind(']', json.find("\"kerning\"", glyphsPos));
        if (arrayStart == std::string::npos)
        {
            return false;
        }
        if (arrayEnd == std::string::npos)
        {
            arrayEnd = json.size();
        }

        size_t pos = arrayStart + 1;
        while (pos < arrayEnd)
        {
            const size_t objStart = json.find('{', pos);
            if (objStart == std::string::npos || objStart >= arrayEnd)
            {
                break;
            }

            // 対応する } を探す(深さ追跡)
            size_t depth = 1;
            size_t objEnd = objStart + 1;
            while (objEnd < json.size() && depth > 0)
            {
                if (json[objEnd] == '{')
                {
                    ++depth;
                }
                else if (json[objEnd] == '}')
                {
                    --depth;
                }
                ++objEnd;
            }

            // このグリフオブジェクトを部分文字列に切り出す
            const std::string glyphJson = json.substr(objStart, objEnd - objStart);

            const int unicode = JsonInt(glyphJson, 0, "unicode");
            if (unicode == 0)
            {
                pos = objEnd;
                continue;
            }

            SDFGlyph glyph{};
            glyph.advance = JsonFloat(glyphJson, 0, "advance");

            const size_t planeBoundsPos = glyphJson.find("\"planeBounds\"");
            if (planeBoundsPos != std::string::npos)
            {
                glyph.planeLeft = JsonFloat(glyphJson, planeBoundsPos, "left");
                glyph.planeBottom = JsonFloat(glyphJson, planeBoundsPos, "bottom");
                glyph.planeRight = JsonFloat(glyphJson, planeBoundsPos, "right");
                glyph.planeTop = JsonFloat(glyphJson, planeBoundsPos, "top");
            }

            const size_t atlasBoundsPos = glyphJson.find("\"atlasBounds\"");
            if (atlasBoundsPos != std::string::npos)
            {
                glyph.atlasLeft = JsonFloat(glyphJson, atlasBoundsPos, "left");
                glyph.atlasBottom = JsonFloat(glyphJson, atlasBoundsPos, "bottom");
                glyph.atlasRight = JsonFloat(glyphJson, atlasBoundsPos, "right");
                glyph.atlasTop = JsonFloat(glyphJson, atlasBoundsPos, "top");
            }

            font.m_glyphs[static_cast<uint32_t>(unicode)] = glyph;
            pos = objEnd;
        }

        return !font.m_glyphs.empty();
    }


    FontId SDFFontEngine::LoadFont(const char* atlasPngPath, const char* atlasJsonPath)
    {
        if (!m_spriteBatch)
        {
            OutputDebugStringA("SDFFontEngine: 初期化されていません。LoadFont() は Initialize() の後に呼んでください。\n");
            return INVALID_FONT_ID;
        }
        if (m_fonts.size() >= MAX_FONT_NUM)
        {
            OutputDebugStringA("SDFFontEngine: 登録できるフォントの数(MAX_FONT_NUM)を超えました。\n");
            return INVALID_FONT_ID;
        }

        FontData font{};
        font.m_emSize = 48.0f;
        font.m_lineHeight = 1.0f;
        font.m_ascender = 0.8f;
        if (!LoadGlyphs(atlasJsonPath, font))
        {
            return INVALID_FONT_ID;
        }

        // アトラス画像をアップロード
        auto* d3dDevice = g_graphicsEngine->GetD3DDevice();
        ResourceUploadBatch uploadBatch(d3dDevice);
        uploadBatch.Begin();
        const std::wstring widePngPath = ToWide(atlasPngPath);
        const HRESULT hr = CreateWICTextureFromFile(
            d3dDevice,
            uploadBatch,
            widePngPath.c_str(),
            font.m_texture.GetAddressOf(),
            false // ミップマップ不要
        );
        auto uploadFuture = uploadBatch.End(g_graphicsEngine->GetCommandQueue());
        uploadFuture.wait();
        if (FAILED(hr) || !font.m_texture)
        {
            OutputDebugStringA("SDFFontEngine: アトラス画像(PNG)が読み込めません。msdf-atlas-gen で生成してください。\n");
            OutputDebugStringA(atlasPngPath);
            OutputDebugStringA("\n");
            return INVALID_FONT_ID;
        }

        // JSONにサイズ情報がなかった場合は、テクスチャのサイズを使う
        const D3D12_RESOURCE_DESC texDesc = font.m_texture->GetDesc();
        if (font.m_atlasWidth == 0)
        {
            font.m_atlasWidth = static_cast<UINT>(texDesc.Width);
        }
        if (font.m_atlasHeight == 0)
        {
            font.m_atlasHeight = texDesc.Height;
        }

        // ディスクリプタヒープの、フォントID番目にSRVを作る
        const FontId fontId = static_cast<FontId>(m_fonts.size());
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
        cpuHandle.ptr += static_cast<SIZE_T>(fontId) * m_srvDescriptorSize;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
        gpuHandle.ptr += static_cast<UINT64>(fontId) * m_srvDescriptorSize;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        d3dDevice->CreateShaderResourceView(font.m_texture.Get(), &srvDesc, cpuHandle);
        font.m_gpuHandle = gpuHandle;

        m_fonts.push_back(std::move(font));
        return fontId;
    }


    float SDFFontEngine::GetEmSize(const FontId fontId) const
    {
        return IsValidFont(fontId) ? m_fonts[fontId].m_emSize : 0.0f;
    }


    //=======================================================================
    // 描画
    //=======================================================================
    void SDFFontEngine::BeginDraw(RenderContext& rc, float rotation, Vector2 gameSpacePos)
    {
        if (!m_spriteBatch)
        {
            return;
        }
        rc.SetDescriptorHeap(m_srvHeap.Get());

        XMMATRIX transform = XMMatrixIdentity();
        if (fabsf(rotation) > 0.0001f)
        {
            // ゲーム座標をスクリーン座標に変換して回転中心とする
            const float pivotX = gameSpacePos.x + UI_SPACE_WIDTH * 0.5f;
            const float pivotY = -gameSpacePos.y + UI_SPACE_HEIGHT * 0.5f;
            transform = XMMatrixTranslation(-pivotX, -pivotY, 0.0f) * XMMatrixRotationZ(rotation) * XMMatrixTranslation(pivotX, pivotY, 0.0f);
        }

        m_spriteBatch->Begin(g_graphicsEngine->GetCommandList(), SpriteSortMode_Deferred, transform);
    }


    void SDFFontEngine::EndDraw([[maybe_unused]] RenderContext& rc)
    {
        if (!m_spriteBatch)
        {
            return;
        }
        m_spriteBatch->End();
    }


    void SDFFontEngine::DrawString(
        const FontData& font,
        const wchar_t* text,
        const Vector2& screenPos,
        const Vector4& color,
        const Vector2& scale,
        const Vector2& pivot
    )
    {
        const float fontSizeX = font.m_emSize * scale.x;
        const float fontSizeY = font.m_emSize * scale.y;

        // ---- 行分割と各行の幅を計算 ----------------------------------------
        struct LineInfo
        {
            const wchar_t* m_start;
            int m_length;
            float m_width;
        };
        LineInfo lines[MAX_LINES];
        int lineCount = 0;

        const wchar_t* lineStart = text;
        for (const wchar_t* ch = text;; ++ch)
        {
            if (*ch != L'\n' && *ch != L'\0')
            {
                continue;
            }

            float lineWidth = 0.0f;
            for (const wchar_t* c = lineStart; c < ch; ++c)
            {
                const auto it = font.m_glyphs.find(static_cast<uint32_t>(*c));
                lineWidth += (it != font.m_glyphs.end()) ? it->second.advance * fontSizeX : fontSizeX * 0.5f;
            }

            if (lineCount < MAX_LINES)
            {
                lines[lineCount++] = { lineStart, static_cast<int>(ch - lineStart), lineWidth };
            }

            if (*ch == L'\0')
            {
                break;
            }
            lineStart = ch + 1;
        }

        if (lineCount == 0)
        {
            return;
        }

        // ---- ブロック全体のサイズ -------------------------------------------
        float maxLineWidth = 0.0f;
        for (int i = 0; i < lineCount; ++i)
        {
            if (lines[i].m_width > maxLineWidth)
            {
                maxLineWidth = lines[i].m_width;
            }
        }

        // totalHeight = 1行目の高さ + (N-1)行分の行間
        const float totalHeight = font.m_lineHeight * fontSizeY * (1.0f + (lineCount - 1) * m_lineSpacing);

        // ブロック左端と最初の行のベースライン
        const float blockLeft = screenPos.x - maxLineWidth * pivot.x;
        const float firstBaseline = screenPos.y + font.m_ascender * fontSizeY - totalHeight * pivot.y;

        const XMUINT2 texSize = { font.m_atlasWidth, font.m_atlasHeight };

        // ---- 行ごとに描画 --------------------------------------------------
        for (int lineIdx = 0; lineIdx < lineCount; ++lineIdx)
        {
            const float lineWidth = lines[lineIdx].m_width;

            float lineStartX;
            switch (m_textAlign)
            {
            case TextAlign::Center:
                lineStartX = blockLeft + (maxLineWidth - lineWidth) * 0.5f;
                break;
            case TextAlign::Right:
                lineStartX = blockLeft + (maxLineWidth - lineWidth);
                break;
            case TextAlign::Left:
            default:
                lineStartX = blockLeft;
                break;
            }

            float cursorX = lineStartX;
            const float baselineY = firstBaseline + lineIdx * font.m_lineHeight * m_lineSpacing * fontSizeY;

            const wchar_t* lineEnd = lines[lineIdx].m_start + lines[lineIdx].m_length;
            for (const wchar_t* c = lines[lineIdx].m_start; c < lineEnd; ++c)
            {
                const auto it = font.m_glyphs.find(static_cast<uint32_t>(*c));
                if (it == font.m_glyphs.end())
                {
                    cursorX += fontSizeX * 0.5f;
                    continue;
                }
                const SDFGlyph& glyph = it->second;

                if (glyph.atlasRight > glyph.atlasLeft && glyph.atlasTop > glyph.atlasBottom)
                {
                    RECT srcRect;
                    srcRect.left = static_cast<LONG>(std::round(glyph.atlasLeft));
                    srcRect.right = static_cast<LONG>(std::round(glyph.atlasRight));
                    srcRect.top = static_cast<LONG>(std::round(font.m_atlasHeight - glyph.atlasTop));
                    srcRect.bottom = static_cast<LONG>(std::round(font.m_atlasHeight - glyph.atlasBottom));

                    RECT dstRect;
                    dstRect.left = static_cast<LONG>(std::round(cursorX + glyph.planeLeft * fontSizeX));
                    dstRect.right = static_cast<LONG>(std::round(cursorX + glyph.planeRight * fontSizeX));
                    dstRect.top = static_cast<LONG>(std::round(baselineY - glyph.planeTop * fontSizeY));
                    dstRect.bottom = static_cast<LONG>(std::round(baselineY - glyph.planeBottom * fontSizeY));

                    m_spriteBatch->Draw(
                        font.m_gpuHandle,
                        texSize,
                        dstRect,
                        &srcRect,
                        XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&color))
                    );
                }

                cursorX += glyph.advance * fontSizeX;
            }
        }
    }


    void SDFFontEngine::Draw(
        const FontId fontId,
        const wchar_t* text,
        const Vector2& position,
        const Vector4& color,
        const Vector2& scale,
        Vector2 pivot
    )
    {
        if (!m_spriteBatch || !IsValidFont(fontId) || text == nullptr)
        {
            return;
        }
        const FontData& font = m_fonts[fontId];

        // 画面の中心が原点(y上が正)の座標を、左上が原点の座標に直す。
        Vector2 screenPos;
        screenPos.x = position.x + UI_SPACE_WIDTH * 0.5f;
        screenPos.y = -position.y + UI_SPACE_HEIGHT * 0.5f;

        pivot.y = 1.0f - pivot.y;

        if (m_isDrawShadow)
        {
            const Vector2 offsets[] = {
                { m_shadowOffset, 0.0f },
                { -m_shadowOffset, 0.0f },
                { 0.0f, m_shadowOffset },
                { 0.0f, -m_shadowOffset },
                { m_shadowOffset, m_shadowOffset },
                { m_shadowOffset, -m_shadowOffset },
                { -m_shadowOffset, m_shadowOffset },
                { -m_shadowOffset, -m_shadowOffset },
            };
            for (const auto& offset : offsets)
            {
                const Vector2 shadowPos = { screenPos.x + offset.x, screenPos.y + offset.y };
                DrawString(font, text, shadowPos, m_shadowColor, scale, pivot);
            }
        }

        DrawString(font, text, screenPos, color, scale, pivot);
    }
} // namespace nsBalloonEngine
