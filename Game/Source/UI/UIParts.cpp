/**
 * @file UIParts.cpp
 * @brief UIのパーツ群
 */
#include "stdafx.h"

#include "UIParts.h"


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
        {}


        UIBase::~UIBase()
        {}




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

            m_sprite.Init(assetName, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

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
        // UIDummy
        //=======================================================================
        UIDummy::UIDummy()
        {}


        UIDummy::~UIDummy()
        {}


        void UIDummy::Update()
        {
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
