/**
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"

#include "TitleScene.h"

#include "DebugScene.h"
#include "Source/UI/Title/TitleMenu.h"


namespace app
{
    namespace
    {
        /** タイトル画面のレイアウトのjson */
        constexpr const char* TITLE_JSON_PATH = "Assets/parameter/ui/Title.json";
        /** スタートを受け付けるパッドの番号 */
        constexpr int PAD_INDEX = 0;
        /** スタートを押した後の、暗転・明転にかける時間(秒) */
        constexpr float FADE_DURATION = 1.0f;
        /** 次のシーン。仮で、InGameScene の中身ができるまでは、遊べる DebugScene に進む */
        constexpr uint32_t NEXT_SCENE_ID = DebugScene::ID();


        /**
         * @brief スタートボタンかAボタンが、このフレームに押されたかどうかを返す
         * @return 押されたらtrue。パッドが無い場合はfalse
         */
        bool IsStartTriggered()
        {
            if (PAD_INDEX < 0 || PAD_INDEX >= static_cast<int>(g_pad.size()) || g_pad[PAD_INDEX] == nullptr)
            {
                return false;
            }

            const GamePad& pad = *g_pad[PAD_INDEX];
            return pad.IsTrigger(enButtonStart) || pad.IsTrigger(enButtonA);
        }
    } // namespace


    TitleScene::TitleScene()
        : m_titlePacket(nullptr)
        , m_isStartRequested(false)
    {
        ui::InitUIPacket(m_titlePacket, TITLE_JSON_PATH);
    }


    TitleScene::~TitleScene()
    {}


    bool TitleScene::Start()
    {
        return true;
    }


    void TitleScene::Update()
    {
        if (!m_isStartRequested && IsStartTriggered())
        {
            m_isStartRequested = true;
        }

        m_titlePacket->Update();
    }


    void TitleScene::PauseUpdate()
    {
        // 暗転している間も、「PRESS START」の点滅を続ける。
        m_titlePacket->Update();
    }


    void TitleScene::Render(RenderContext& rc)
    {}


    void TitleScene::RenderUI(RenderContext& rc)
    {
        m_titlePacket->Render(rc);
    }


    bool TitleScene::RequestScene(uint32_t& id, float& waitTime)
    {
        if (!m_isStartRequested)
        {
            return false;
        }

        id = NEXT_SCENE_ID;
        waitTime = FADE_DURATION;
        return true;
    }
} // namespace app
