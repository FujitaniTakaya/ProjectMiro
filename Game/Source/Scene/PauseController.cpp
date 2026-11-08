/**
 * @file PauseController.cpp
 * @brief ポーズ画面の流れを動かすクラス
 */
#include "stdafx.h"

#include "PauseController.h"

#include <algorithm>

#include "Source/UI/Option/OptionMenu.h"
#include "Source/UI/Pause/PauseMenu.h"


namespace app
{
    namespace
    {
        /** ポーズ画面のレイアウトのjson */
        constexpr const char* PAUSE_JSON_PATH = "Assets/parameter/ui/Pause.json";
        /** オプション画面のレイアウトのjson */
        constexpr const char* OPTION_JSON_PATH = "Assets/parameter/ui/Option.json";
        /** 入力を受け付けるパッドの番号 */
        constexpr int PAD_INDEX = 0;
        /** ぼけ具合が 0 から 1 まで(1 から 0 まで)動く時間(秒) */
        constexpr float FADE_TIME = 0.25f;
    } // namespace


    PauseController::PauseController()
        : m_menuPacket(nullptr)
        , m_optionPacket(nullptr)
        , m_menuInput()
        , m_state(PauseState::Closed)
        , m_fade(0.0f)
    {
        ui::InitUIPacket(m_menuPacket, PAUSE_JSON_PATH);
        ui::InitUIPacket(m_optionPacket, OPTION_JSON_PATH);

        ApplyFade();
    }


    PauseController::~PauseController()
    {}


    bool PauseController::IsOpenTriggered()
    {
        const GamePad* pad = ui::MenuInput::FindPad(PAD_INDEX);
        return pad && pad->IsTrigger(enButtonStart);
    }


    void PauseController::Open()
    {
        if (m_state != PauseState::Closed)
        {
            return;
        }

        m_state = PauseState::Opening;
        m_fade = 0.0f;
        m_menuInput.Reset();
        // NOTE: 呼んだフレームのうちに描かれても、前回の選択や、jsonの値のままの見た目が映らないよう、すぐ反映する。
        //       この後すぐに Update() は呼ばない。(同じフレームのスタートボタンを、項目の操作として受け取ってしまう)
        if (ui::PauseMenu* menu = m_menuPacket->GetMenu())
        {
            menu->Open();
        }
        ApplyFade();
    }


    void PauseController::Update()
    {
        if (m_state == PauseState::Closed)
        {
            return;
        }

        const float deltaTime = g_gameTime->GetFrameDeltaTime();

        switch (m_state)
        {
        case PauseState::Opening:
            OpeningUpdate(deltaTime);
            break;
        case PauseState::Menu:
            MenuUpdate(deltaTime);
            break;
        case PauseState::Option:
            OptionUpdate();
            break;
        case PauseState::Closing:
            ClosingUpdate(deltaTime);
            break;
        default:
            break;
        }

        ApplyFade();

        // オプション画面を開いている間は、ポーズ画面は描かないので更新しない。(オプション画面は OptionUpdate() で更新している)
        if (m_state != PauseState::Closed && m_state != PauseState::Option)
        {
            m_menuPacket->Update();
        }
    }


    void PauseController::RenderUI(RenderContext& rc)
    {
        switch (m_state)
        {
        case PauseState::Opening:
        case PauseState::Menu:
        case PauseState::Closing:
        case PauseState::ToTitle:
            m_menuPacket->Render(rc);
            break;
        case PauseState::Option:
            m_optionPacket->Render(rc);
            break;
        default:
            break;
        }
    }


    void PauseController::Reset()
    {
        m_state = PauseState::Closed;
        m_fade = 0.0f;
        m_menuInput.Reset();
        ApplyFade();
    }


    void PauseController::OpeningUpdate(const float deltaTime)
    {
        m_fade = std::clamp(m_fade + deltaTime / FADE_TIME, 0.0f, 1.0f);
        if (m_fade >= 1.0f)
        {
            m_state = PauseState::Menu;
            m_menuInput.Reset();
        }
    }


    void PauseController::MenuUpdate(const float deltaTime)
    {
        const GamePad* pad = ui::MenuInput::FindPad(PAD_INDEX);
        ui::PauseMenu* menu = m_menuPacket->GetMenu();
        if (!pad || !menu)
        {
            return;
        }

        m_menuInput.Update(*pad, deltaTime);

        const int move = m_menuInput.GetMoveY();
        if (move != 0)
        {
            menu->MoveSelection(move);
        }

        if (pad->IsTrigger(enButtonB) || pad->IsTrigger(enButtonStart))
        {
            m_state = PauseState::Closing;
        }
        else if (pad->IsTrigger(enButtonA))
        {
            Decide(menu->GetSelectedItem());
        }
    }


    void PauseController::OptionUpdate()
    {
        m_optionPacket->Update();

        // 画面が閉じられたら、ポーズ画面に戻る。(Menuが無い場合も、戻れなくならないように戻す)
        const ui::OptionMenu* menu = m_optionPacket->GetMenu();
        if (!menu || menu->IsClosed())
        {
            m_state = PauseState::Menu;
            m_menuInput.Reset();
        }
    }


    void PauseController::ClosingUpdate(const float deltaTime)
    {
        m_fade = std::clamp(m_fade - deltaTime / FADE_TIME, 0.0f, 1.0f);

        // NOTE: 押したままゲームに戻ると、ダッシュ(B)などが、戻った瞬間に押されたことになってしまう。はなされるまで待つ。
        const GamePad* pad = ui::MenuInput::FindPad(PAD_INDEX);
        const bool isReleased = !pad || (!pad->IsPress(enButtonB) && !pad->IsPress(enButtonStart));
        if (m_fade <= 0.0f && isReleased)
        {
            m_state = PauseState::Closed;
            m_menuInput.Reset();
        }
    }


    void PauseController::Decide(const ui::EnPauseItem item)
    {
        switch (item)
        {
        case ui::EnPauseItem::Option:
            m_state = PauseState::Option;
            m_menuInput.Reset();
            if (ui::OptionMenu* menu = m_optionPacket->GetMenu())
            {
                menu->Open();
            }
            break;
        case ui::EnPauseItem::Title:
            m_state = PauseState::ToTitle;
            break;
        default:
            break;
        }
    }


    void PauseController::ApplyFade()
    {
        RenderingEngine::Get().SetSceneBlurRate(m_fade);

        if (ui::PauseMenu* menu = m_menuPacket->GetMenu())
        {
            menu->SetAlpha(m_fade);
        }
    }
} // namespace app
