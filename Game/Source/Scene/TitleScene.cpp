/**
 * @file TitleScene.cpp
 * @brief タイトルシーン
 */
#include "stdafx.h"

#include "TitleScene.h"

#include "DebugScene.h"
#include "Source/UI/Screen/Option/OptionMenu.h"
#include "Source/UI/Screen/Title/TitleMenu.h"


namespace app
{
    namespace
    {
        /** タイトル画面のレイアウトのjson */
        constexpr const char* TITLE_JSON_PATH = "Assets/parameter/ui/Title.json";
        /** オプション画面のレイアウトのjson */
        constexpr const char* OPTION_JSON_PATH = "Assets/parameter/ui/Option.json";
        /** 入力を受け付けるパッドの番号 */
        constexpr int PAD_INDEX = 0;
        /** スタートを押した後の、暗転・明転にかける時間(秒) */
        constexpr float FADE_DURATION = 1.0f;
        /** 次のシーン。仮で、InGameScene の中身ができるまでは、遊べる DebugScene に進む */
        constexpr uint32_t NEXT_SCENE_ID = DebugScene::ID();
    } // namespace


    TitleScene::TitleScene()
        : m_titlePacket(nullptr)
        , m_optionPacket(nullptr)
        , m_menuInput()
        , m_backdrop()
        , m_state(TitleState::Title)
        , m_isCapturePending(false)
        , m_isStartRequested(false)
    {
        ui::InitUIPacket(m_titlePacket, TITLE_JSON_PATH);
        ui::InitUIPacket(m_optionPacket, OPTION_JSON_PATH);
    }


    TitleScene::~TitleScene()
    {}


    bool TitleScene::Start()
    {
        return true;
    }


    void TitleScene::Update()
    {
        switch (m_state)
        {
        case TitleState::Title:
            TitleUpdate();
            break;
        case TitleState::Option:
            OptionUpdate();
            break;
        default:
            break;
        }
    }


    void TitleScene::PauseUpdate()
    {
        // 暗転している間も、UIの更新を続ける。(暗転に入るのは、タイトル画面で「はじめる」を決めた時だけ)
        m_titlePacket->Update();
    }


    void TitleScene::Render(RenderContext& rc)
    {}


    void TitleScene::RenderUI(RenderContext& rc)
    {
        switch (m_state)
        {
        case TitleState::Title:
            m_titlePacket->Render(rc);
            break;
        case TitleState::Option:
            // 決めたフレームの描画で、直前のタイトル画面を取り込んでぼかす。(フェードは、この後にApplicationが描くので映らない)
            if (m_isCapturePending)
            {
                m_backdrop.Capture(rc, [this](RenderContext& context) { m_titlePacket->Render(context); });
                m_isCapturePending = false;
            }
            m_backdrop.Render(rc);
            m_optionPacket->Render(rc);
            break;
        default:
            break;
        }
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


    void TitleScene::TitleUpdate()
    {
        const GamePad* pad = ui::MenuInput::FindPad(PAD_INDEX);
        ui::TitleMenu* menu = m_titlePacket->GetMenu();

        // 「はじめる」が決まった後は、暗転が終わるまで項目を動かさない。
        if (pad && menu && !m_isStartRequested)
        {
            m_menuInput.Update(*pad, g_gameTime->GetFrameDeltaTime());

            const int move = m_menuInput.GetMoveY();
            if (move != 0)
            {
                menu->MoveSelection(move);
            }

            if (pad->IsTrigger(enButtonA) || pad->IsTrigger(enButtonStart))
            {
                Decide(menu->GetSelectedItem());
            }
        }

        m_titlePacket->Update();
    }


    void TitleScene::OptionUpdate()
    {
        m_optionPacket->Update();

        // 画面が閉じられたら、タイトル画面に戻る。(Menuが無い場合も、戻れなくならないように戻す)
        const ui::OptionMenu* menu = m_optionPacket->GetMenu();
        if (!menu || menu->IsClosed())
        {
            m_state = TitleState::Title;
            m_menuInput.Reset();
        }
    }


    void TitleScene::Decide(const ui::EnTitleItem item)
    {
        switch (item)
        {
        case ui::EnTitleItem::Start:
            m_isStartRequested = true;
            break;
        case ui::EnTitleItem::Option:
            m_state = TitleState::Option;
            m_isCapturePending = true;
            m_menuInput.Reset();
            if (ui::OptionMenu* menu = m_optionPacket->GetMenu())
            {
                menu->Open();
            }
            break;
        case ui::EnTitleItem::Quit:
            // WM_QUITを送って、ゲームループを抜けさせる。(ウィンドウの×ボタンで閉じた時と同じ後始末になる。)
            PostQuitMessage(0);
            break;
        default:
            break;
        }
    }
} // namespace app
