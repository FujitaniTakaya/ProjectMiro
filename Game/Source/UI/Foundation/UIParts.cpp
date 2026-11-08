/**
 * @file UIParts.cpp
 * @brief UIのパーツ群
 */
#include "stdafx.h"

#include "UIParts.h"

#include <cmath>
#include <string>


namespace app
{
    namespace ui
    {
        //=======================================================================
        // UIBase
        //=======================================================================
        // NOTE: 色と基点は、Vector4::Whiteなどのstatic変数ではなく、リテラルで初期化している。
        //       他のファイルのstatic変数の初期化中に生成された場合に、初期化の順番によってはゼロのままコピーされてしまうため。
        UIBase::UIBase()
            : m_transform()
            , m_color(1.0f, 1.0f, 1.0f, 1.0f)
            , m_pivot(0.5f, 0.5f)
            , m_isDraw(true)
            , m_key(0)
            , m_animations()
        {}


        UIBase::~UIBase()
        {}


        void UIBase::AddAnimation(const uint32_t key, std::unique_ptr<UIAnimationBase> animation)
        {
            if (!animation)
            {
                return;
            }
            animation->SetUI(this);

            for (AnimationEntry& entry : m_animations)
            {
                if (entry.m_key == key)
                {
                    entry.m_animation = std::move(animation);
                    return;
                }
            }
            m_animations.emplace_back(key, std::move(animation));
        }


        void UIBase::RemoveAnimation(const uint32_t key)
        {
            const auto it = std::find_if(
                m_animations.begin(),
                m_animations.end(),
                [key](const AnimationEntry& entry) { return entry.m_key == key; }
            );
            if (it != m_animations.end())
            {
                m_animations.erase(it);
            }
        }




        //=======================================================================
        // UIImage
        //=======================================================================
        UIImage::UIImage()
            : m_sprite()
        {}


        UIImage::~UIImage()
        {}


        void UIImage::Update()
        {
            // アニメーションはローカルのトランスフォームを動かすので、トランスフォームの更新より先に更新する。
            UpdateAnimation();

            // 親を考慮したワールドのトランスフォームで描画する。
            m_transform.UpdateTransform();

            const Transform& world = m_transform.m_worldTransform;
            m_sprite.SetPosition(world.m_position);
            m_sprite.SetRotation(world.m_rotation);
            m_sprite.SetScale(world.m_scale);
            m_sprite.SetPivot(m_pivot);
            m_sprite.SetMulColor(m_color);
            m_sprite.Update();
        }


        void UIImage::Render(RenderContext& rc)
        {
            if (m_isDraw)
            {
                m_sprite.Draw(rc);
            }
        }


        void UIImage::InitializeImage(
            const char* assetName,
            const float width,
            const float height,
            const Vector3& position,
            const Vector3& scale,
            const Quaternion& rotation,
            const Vector4& color,
            const Vector2& pivot
        )
        {
            m_transform.m_localTransform.m_position = position;
            m_transform.m_localTransform.m_scale = scale;
            m_transform.m_localTransform.m_rotation = rotation;
            m_color = color;
            m_pivot = pivot;

            m_sprite.Init(assetName, static_cast<uint32_t>(width), static_cast<uint32_t>(height), SpriteRender::DEFAULT_FX_PATH, AlphaBlendMode_Trans);

            // 初期化した直後から、正しい位置に描画できるようにする。
            Update();
        }




        //=======================================================================
        // UIIcon
        //=======================================================================
        UIIcon::UIIcon()
            : m_size(0.0f, 0.0f)
        {}


        UIIcon::~UIIcon()
        {}


        void UIIcon::Initialize(
            const char* assetName,
            const float width,
            const float height,
            const Vector3& position,
            const Vector3& scale,
            const Quaternion& rotation,
            const Vector4& color
        )
        {
            m_size = Vector2(width, height);
            InitializeImage(assetName, width, height, position, scale, rotation, color, m_pivot);
        }




        //=======================================================================
        // UIButton
        //=======================================================================
        UIButton::UIButton()
        {}


        UIButton::~UIButton()
        {}


        void UIButton::Initialize(
            const char* assetName,
            const float width,
            const float height,
            const Vector3& position,
            const Vector3& scale,
            const Quaternion& rotation,
            const Vector4& color
        )
        {
            InitializeImage(assetName, width, height, position, scale, rotation, color, m_pivot);
        }




        //=======================================================================
        // UIGauge
        //=======================================================================
        UIGauge::UIGauge()
        {}


        UIGauge::~UIGauge()
        {}


        void UIGauge::Initialize(
            const char* assetName,
            const float width,
            const float height,
            const Vector3& position,
            const Vector3& scale,
            const Quaternion& rotation,
            const Vector4& color,
            const Vector2& pivot
        )
        {
            InitializeImage(assetName, width, height, position, scale, rotation, color, pivot);
        }




        //=======================================================================
        // UIText
        //=======================================================================
        namespace
        {
            /**
             * @brief クォータニオンの、Z軸回りの回転角を求める
             * @details Z軸回りだけの回転を前提にする。(UIの回転は、Z軸回りだけ。)
             * @param rotation 回転
             * @return Z軸回りの回転角(ラジアン)。反時計回りが正。
             */
            float ToZRadian(const Quaternion& rotation)
            {
                return 2.0f * std::atan2(rotation.z, rotation.w);
            }
        } // namespace


        UIText::InitData::InitData()
            : m_fontId(INVALID_FONT_ID)
            , m_text()
            , m_fontSize(0.0f)
            , m_align(TextAlign::Left)
            , m_lineSpacing(1.2f)
            , m_shadowOffset(0.0f)
            , m_shadowColor(0.0f, 0.0f, 0.0f, 1.0f)
            , m_position(0.0f, 0.0f, 0.0f)
            , m_scale(1.0f, 1.0f, 1.0f)
            , m_rotation()
            , m_color(1.0f, 1.0f, 1.0f, 1.0f)
            , m_pivot(0.5f, 0.5f)
        {}


        UIText::UIText()
            : m_fontRender()
            , m_fontSize(0.0f)
        {}


        UIText::~UIText()
        {}


        void UIText::Update()
        {
            // アニメーションはローカルのトランスフォームを動かすので、トランスフォームの更新より先に更新する。
            UpdateAnimation();

            // 親を考慮したワールドのトランスフォームで描画する。
            m_transform.UpdateTransform();
            const Transform& world = m_transform.m_worldTransform;

            // フォントサイズ(ピクセル)を、フォントの標準の大きさ(em)に対する倍率に直す。
            float baseScale = 1.0f;
            if (m_fontSize > 0.0f)
            {
                const float em = SDFFontEngine::Get().GetEmSize(m_fontRender.GetFont());
                baseScale = (em > 0.0f) ? m_fontSize / em : 0.0f;
            }

            m_fontRender.SetPosition(world.m_position.x, world.m_position.y);
            m_fontRender.SetScale(Vector2(world.m_scale.x * baseScale, world.m_scale.y * baseScale));
            // NOTE: FontRenderの回転は、画面の上下が逆の座標で回すので、時計回りが正。UIの回転(反時計回りが正)に合わせるため、符号を反転する。
            m_fontRender.SetRotation(-ToZRadian(world.m_rotation));
            m_fontRender.SetPivot(m_pivot);
            m_fontRender.SetColor(m_color);
        }


        void UIText::Render(RenderContext& rc)
        {
            if (m_isDraw)
            {
                m_fontRender.Draw(rc);
            }
        }


        void UIText::Initialize(const InitData& data)
        {
            m_transform.m_localTransform.m_position = data.m_position;
            m_transform.m_localTransform.m_scale = data.m_scale;
            m_transform.m_localTransform.m_rotation = data.m_rotation;
            m_color = data.m_color;
            m_pivot = data.m_pivot;
            m_fontSize = data.m_fontSize;

            m_fontRender.SetFont(data.m_fontId);
            m_fontRender.SetText(data.m_text.c_str());
            m_fontRender.SetTextAlign(data.m_align);
            m_fontRender.SetLineSpacing(data.m_lineSpacing);
            m_fontRender.SetShadowParam(data.m_shadowOffset > 0.0f, data.m_shadowOffset, data.m_shadowColor);

            // 初期化した直後から、正しい位置に描画できるようにする。
            Update();
        }




        //=======================================================================
        // UIDummy
        //=======================================================================
        UIDummy::UIDummy()
        {}


        UIDummy::~UIDummy()
        {}


        void UIDummy::Update()
        {
            UpdateAnimation();
            m_transform.UpdateTransform();
        }


        void UIDummy::Render(RenderContext& rc)
        {
            // 何も描画しない。
        }




        //=======================================================================
        // UICanvas
        //=======================================================================
        UICanvas::UICanvas()
            : m_uiList()
        {}


        UICanvas::~UICanvas()
        {}


        void UICanvas::Update()
        {
            UpdateAnimation();

            // キャンバスのトランスフォームを更新する。(子のワールドのトランスフォームも更新される。)
            m_transform.UpdateTransform();

            for (auto& ui : m_uiList)
            {
                ui->Update();
            }
        }


        void UICanvas::Render(RenderContext& rc)
        {
            for (auto& ui : m_uiList)
            {
                ui->Render(rc);
            }
        }


        void UICanvas::RemoveUI(const uint32_t key)
        {
            const auto it = std::find_if(
                m_uiList.begin(),
                m_uiList.end(),
                [key](const std::unique_ptr<UIBase>& ui) { return ui->GetKey() == key; }
            );
            if (it != m_uiList.end())
            {
                m_uiList.erase(it);
            }
        }
    } // namespace ui
} // namespace app
