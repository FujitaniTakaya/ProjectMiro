/**
 * @file UISprite.cpp
 * @brief UIの画像1枚を描画するクラス
 */
#include "stdafx.h"

#include "UISprite.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** スプライトのシェーダー */
            constexpr const char* SPRITE_SHADER_PATH = "Assets/shader/balloon/sprite.fx";
        } // namespace


        UISprite::UISprite()
            : m_sprite()
            , m_transform()
            , m_pivot(0.5f, 0.5f)
        {}


        UISprite::~UISprite()
        {}


        void UISprite::Init(
            const char* ddsFilePath,
            const uint32_t width,
            const uint32_t height,
            const AlphaBlendMode alphaBlendMode
        )
        {
            SpriteInitData initData;
            initData.m_ddsFilePath.at(0) = ddsFilePath;
            initData.m_fxFilePath = SPRITE_SHADER_PATH;
            initData.m_width = width;
            initData.m_height = height;
            initData.m_alphaBlendMode = alphaBlendMode;

            m_sprite.Init(initData);
        }


        void UISprite::Update()
        {
            m_sprite.Update(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale, m_pivot);
        }


        void UISprite::Draw(RenderContext& rc)
        {
            m_sprite.Draw(rc);
        }


        void UISprite::SetPosition(const Vector3& position)
        {
            m_transform.m_position = position;
        }


        void UISprite::SetRotation(const Quaternion& rotation)
        {
            m_transform.m_rotation = rotation;
        }


        void UISprite::SetScale(const Vector3& scale)
        {
            m_transform.m_scale = scale;
        }


        void UISprite::SetPivot(const Vector2& pivot)
        {
            m_pivot = pivot;
        }


        void UISprite::SetMulColor(const Vector4& mulColor)
        {
            // NOTE: 乗算色は、Draw()の時にSpriteから読まれるので、Update()を呼ばなくても反映される。
            m_sprite.SetMulColor(mulColor);
        }
    } // namespace ui
} // namespace app
