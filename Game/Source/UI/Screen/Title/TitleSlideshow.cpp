/**
 * @file TitleSlideshow.cpp
 * @brief タイトル画面の背景。インゲームのスクリーンショットを順に流す
 */
#include "stdafx.h"

#include "TitleSlideshow.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include <utility>


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 画像を置くフォルダ。末尾は「/」 */
            const std::string SLIDE_DIRECTORY = "Assets/spriteData/UI/Title/";
            /** 画像の拡張子(小文字) */
            constexpr const char* SLIDE_EXTENSION = ".dds";
            /** 下地の画像(白)。乗算色で色を付ける */
            constexpr const char* BASE_SPRITE_PATH = "Assets/spriteData/UI/White.DDS";
            /** 下地の色。画像が1枚も無い時に見える色 */
            const Vector4 BASE_COLOR(18.0f / 255.0f, 22.0f / 255.0f, 34.0f / 255.0f, 1.0f);
            /** 次の画像に切り替え始めるまで、1枚を全部見せておく時間(秒) */
            constexpr float SHOW_TIME = 6.5f;
            /** 切り替え(次の画像が透明から不透明になる)にかける時間(秒) */
            constexpr float FADE_TIME = 1.5f;
            /** 1枚の画像が出ている間に増える拡大率。1枚は、(SHOW_TIME + FADE_TIME * 2)秒かけて、等倍からこの分だけ拡大する */
            constexpr float ZOOM_RATE = 0.06f;
            /** 1枚の画像の寿命(秒)。自分が切り替わって出る時間 + 全部見せる時間 + 次の画像に覆われていく時間 */
            constexpr float SLIDE_LIFE = SHOW_TIME + FADE_TIME * 2.0f;


            /**
             * @brief フォルダにある画像のパスを、ファイル名の順に集める
             * @details フォルダが無い時や、dds が無い時は、空を返す。ファイル名は半角英数字にすること。
             * @return 画像のパス
             */
            std::vector<std::string> FindSlideImages()
            {
                std::vector<std::string> paths;

                std::error_code error;
                std::filesystem::directory_iterator it(SLIDE_DIRECTORY, error);
                for (; !error && it != std::filesystem::directory_iterator(); it.increment(error))
                {
                    if (!it->is_regular_file(error))
                    {
                        continue;
                    }

                    std::string extension = it->path().extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    if (extension == SLIDE_EXTENSION)
                    {
                        paths.push_back(SLIDE_DIRECTORY + it->path().filename().string());
                    }
                }

                std::sort(paths.begin(), paths.end());
                return paths;
            }
        } // namespace


        TitleSlideshow::TitleSlideshow()
            : m_base()
            , m_slides()
            , m_current(0)
            , m_timer(0.0f)
        {
            m_base.Init(BASE_SPRITE_PATH, FRAME_BUFFER_W, FRAME_BUFFER_H);
            m_base.SetMulColor(BASE_COLOR);
            m_base.Update();

            // NOTE: アルファブレンドをAlphaBlendMode_Transにしないと、乗算色のアルファが効かず、切り替えがフェードにならない。
            for (const std::string& path : FindSlideImages())
            {
                std::unique_ptr<SpriteRender> slide = std::make_unique<SpriteRender>();
                slide->Init(path.c_str(), FRAME_BUFFER_W, FRAME_BUFFER_H, SpriteRender::DEFAULT_FX_PATH, AlphaBlendMode_Trans);
                m_slides.push_back(std::move(slide));
            }
        }


        TitleSlideshow::~TitleSlideshow() = default;


        void TitleSlideshow::Update(const float deltaTime)
        {
            if (m_slides.empty())
            {
                return;
            }

            m_timer += deltaTime;

            // 1枚だけの時は、切り替えない。拡大が最後まで進んだら、そのまま止める。
            if (m_slides.size() == 1)
            {
                m_timer = (std::min)(m_timer, SLIDE_LIFE);
                return;
            }

            // 切り替えが終わったら、次の画像を「今の画像」にする。
            // 次の画像は、切り替えの間(FADE_TIME秒)に、もう拡大が進んでいる。時間を巻き戻さず、その分を引き継ぐ。
            if (m_timer >= SHOW_TIME + FADE_TIME)
            {
                m_timer -= SHOW_TIME;
                m_current = (m_current + 1) % m_slides.size();
            }
        }


        void TitleSlideshow::Render(RenderContext& rc)
        {
            m_base.Draw(rc);

            if (m_slides.empty())
            {
                return;
            }

            // 下に、今の画像を不透明で描く。
            DrawSlide(m_current, m_timer, 1.0f, rc);

            // 切り替えの間は、上に、次の画像を透明から重ねる。
            if (m_slides.size() > 1 && m_timer > SHOW_TIME)
            {
                const float fadeTime = m_timer - SHOW_TIME;
                const float alpha = std::clamp(fadeTime / FADE_TIME, 0.0f, 1.0f);
                DrawSlide((m_current + 1) % m_slides.size(), fadeTime, alpha, rc);
            }
        }


        void TitleSlideshow::DrawSlide(const size_t index, const float localTime, const float alpha, RenderContext& rc)
        {
            // 画面の中心を基点に拡大するので、拡大しても画面の端は空かない。
            const float scale = 1.0f + ZOOM_RATE * std::clamp(localTime / SLIDE_LIFE, 0.0f, 1.0f);

            SpriteRender& slide = *m_slides[index];
            slide.SetScale(Vector3(scale, scale, 1.0f));
            slide.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, alpha));
            slide.Update();
            slide.Draw(rc);
        }
    } // namespace ui
} // namespace app
