/**
 * @file Fade.cpp
 * @brief 画面の暗転・明転をするクラス
 */
#include "stdafx.h"

#include "Fade.h"


namespace app
{
    namespace
    {
        /** 暗幕の画像 */
        constexpr const char* FADE_SPRITE_PATH = "Assets/spriteData/UI/Load/Load.DDS";
        /** スプライトのシェーダー */
        constexpr const char* SPRITE_SHADER_PATH = "Assets/shader/balloon/sprite.fx";
    } // namespace


    Fade* Fade::m_instance = nullptr;


    Fade::Fade()
        : m_fadeSprite()
        , m_state(FadeState::None)
        , m_timer(0.0f)
        , m_duration(0.0f)
    {
        // NOTE: SpriteRenderは、アルファブレンドの指定ができない(AlphaBlendMode_None固定)ので、Spriteを直接使う。
        //       アルファブレンドが無いと、乗算色のアルファが効かず、フェードにならない。
        SpriteInitData initData;
        initData.m_ddsFilePath.at(0) = FADE_SPRITE_PATH;
        initData.m_fxFilePath = SPRITE_SHADER_PATH;
        initData.m_width = FRAME_BUFFER_W;
        initData.m_height = FRAME_BUFFER_H;
        initData.m_alphaBlendMode = AlphaBlendMode_Trans;
        m_fadeSprite.Init(initData);

        // 暗幕は動かないので、一度だけ更新する。
        m_fadeSprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
    }


    Fade::~Fade()
    {}


    void Fade::Update()
    {
        const float deltaTime = g_gameTime->GetFrameDeltaTime();

        if (m_state == FadeState::FadeIn)
        {
            m_timer -= deltaTime;
            if (m_timer <= 0.0f)
            {
                m_timer = 0.0f;
                m_state = FadeState::None;
            }
        }
        else if (m_state == FadeState::FadeOut)
        {
            // 完了(m_timer == m_duration)しても、FadeInが始まるまでは状態をFadeOutのままにする。(IsFadeOutComplete()で判定する)
            m_timer = (std::min)(m_timer + deltaTime, m_duration);
        }
    }


    void Fade::Render(RenderContext& rc)
    {
        if (m_state == FadeState::None)
        {
            return;
        }

        m_fadeSprite.SetMulColor({ 1.0f, 1.0f, 1.0f, CalcAlpha() });
        m_fadeSprite.Draw(rc);
    }


    void Fade::FadeOut(float duration)
    {
        m_state = FadeState::FadeOut;
        m_duration = duration;
        m_timer = 0.0f;
    }


    void Fade::FadeIn(float duration)
    {
        m_state = FadeState::FadeIn;
        m_duration = duration;
        m_timer = duration;
    }


    float Fade::CalcAlpha() const
    {
        // 時間が0以下のときは、割り算をせずに、フェードの向きだけで決める。(0除算でNaNになるのを防ぐ)
        if (m_duration <= 0.0f)
        {
            return (m_state == FadeState::FadeOut) ? 1.0f : 0.0f;
        }
        return std::clamp(m_timer / m_duration, 0.0f, 1.0f);
    }


    void Fade::CreateInstance()
    {
        K2_ASSERT(m_instance == nullptr, "Fadeは既に生成されている。");
        if (m_instance == nullptr)
        {
            m_instance = new Fade();
        }
    }


    void Fade::DestroyInstance()
    {
        delete m_instance;
        m_instance = nullptr;
    }


    Fade& Fade::Get()
    {
        K2_ASSERT(m_instance != nullptr, "Fade::CreateInstance()を先に呼ぶこと。");
        return *m_instance;
    }
}
