/**
 * @file UIParts.cpp
 * @brief UIのパーツ群
 */
#include "stdafx.h"

#include "UIParts.h"

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
        // UIDigit
        //=======================================================================
        UIDigit::UIDigit()
            : m_maxDigits(1)
            , m_maxNumber(0)
            , m_number(0)
            , m_visibleDigits(1)
            , m_digits()
            , m_digitWidth(0.0f)
            , m_digitSprites(nullptr)
        {}


        UIDigit::~UIDigit()
        {}


        void UIDigit::Update()
        {
            UpdateAnimation();

            // 親を考慮したワールドのトランスフォームで描画する。
            m_transform.UpdateTransform();
            if (!m_digitSprites)
            {
                return;
            }

            const Transform& world = m_transform.m_worldTransform;
            for (uint32_t i = 0; i < m_visibleDigits; ++i)
            {
                // 上の桁ほど左へ。拡大・回転も掛ける。
                Vector3 offset(-m_digitWidth * world.m_scale.x * static_cast<float>(i), 0.0f, 0.0f);
                world.m_rotation.Apply(offset);

                UISprite& sprite = m_digitSprites[i * DIGIT_KIND_COUNT + m_digits[i]];
                sprite.SetPosition(world.m_position + offset);
                sprite.SetRotation(world.m_rotation);
                sprite.SetScale(world.m_scale);
                sprite.SetPivot(m_pivot);
                sprite.SetMulColor(m_color);
                sprite.Update();
            }
        }


        void UIDigit::Render(RenderContext& rc)
        {
            if (!m_isDraw || !m_digitSprites)
            {
                return;
            }

            for (uint32_t i = 0; i < m_visibleDigits; ++i)
            {
                m_digitSprites[i * DIGIT_KIND_COUNT + m_digits[i]].Draw(rc);
            }
        }


        void UIDigit::Initialize(
            const char* folderPath,
            const uint32_t maxDigits,
            const int number,
            const float width,
            const float height,
            const Vector3& position,
            const Vector3& scale,
            const Quaternion& rotation,
            const Vector4& color
        )
        {
            m_transform.m_localTransform.m_position = position;
            m_transform.m_localTransform.m_scale = scale;
            m_transform.m_localTransform.m_rotation = rotation;
            m_color = color;
            m_digitWidth = width;

            m_maxDigits = std::clamp<uint32_t>(maxDigits, 1, MAX_DIGITS);
            m_maxNumber = 0;
            for (uint32_t i = 0; i < m_maxDigits; ++i)
            {
                m_maxNumber = m_maxNumber * 10 + 9;
            }

            // 桁ごとに、0〜9の画像を全て読み込む。("フォルダ/0.dds" の0を、数字に書き換えていく。)
            m_digitSprites = std::make_unique<UISprite[]>(m_maxDigits * DIGIT_KIND_COUNT);
            std::string path = std::string(folderPath) + "/0.dds";
            const size_t numberIndex = path.size() - 5;
            for (uint32_t position = 0; position < m_maxDigits; ++position)
            {
                for (uint32_t digit = 0; digit < DIGIT_KIND_COUNT; ++digit)
                {
                    path[numberIndex] = static_cast<char>('0' + digit);
                    m_digitSprites[position * DIGIT_KIND_COUNT + digit].Init(
                        path.c_str(),
                        static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)
                    );
                }
            }

            SetNumber(number);

            // 初期化した直後から、正しい位置に描画できるようにする。
            Update();
        }


        void UIDigit::SetNumber(const int number)
        {
            m_number = std::clamp(number, 0, m_maxNumber);

            // 1の位から順に、桁の数字を取り出す。先頭の0は表示しないが、1桁目は必ず表示する。
            m_visibleDigits = 1;
            int rest = m_number;
            for (uint32_t i = 0; i < m_digits.size(); ++i)
            {
                m_digits[i] = static_cast<uint8_t>(rest % 10);
                rest /= 10;
                if (rest > 0)
                {
                    m_visibleDigits = i + 2;
                }
            }
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
