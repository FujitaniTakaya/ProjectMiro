/**
 * @file ResultScene.cpp
 * @brief リザルトシーン
 */
#include "stdafx.h"

#include "ResultScene.h"

#include "Fade.h"
#include "Source/Score/GameResult.h"
#include "Source/Score/ScoreRule.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Screen/Result/ResultMenu.h"
#include "TitleScene.h"


namespace app
{
    namespace
    {
        /** リザルト画面のレイアウトのjson */
        constexpr const char* RESULT_JSON_PATH = "Assets/parameter/ui/Result.json";
        /** 入力を受け付けるパッドの番号 */
        constexpr int PAD_INDEX = 0;
        /** 「タイトルへ」を決めた後の、暗転・明転にかける時間(秒)。タイトルからインゲームへ進む時と同じ */
        constexpr float FADE_DURATION = 1.0f;
        /** リザルトBGMのフェードインにかける時間(秒)。インゲームからの明転と同じ */
        constexpr float BGM_FADE_IN_SECONDS = 1.0f;
        /** 次のシーン。「タイトルへ」を決めたら、タイトルシーンに進む */
        constexpr uint32_t NEXT_SCENE_ID = TitleScene::ID();
    } // namespace


    ResultScene::ResultScene()
        : m_resultPacket(nullptr)
        , m_menuInput()
        , m_isTitleRequested(false)
    {
        ui::InitUIPacket(m_resultPacket, RESULT_JSON_PATH);
    }


    ResultScene::~ResultScene()
    {}


    bool ResultScene::Start()
    {
        // Start()は暗転中に呼ばれるので、フェードインが明転にそのまま重なる。インゲームBGMは、InGameScene側で消え終わっている
        SoundManager::Get().GetBGM().Play(EnSoundID::BgmResult, DEFAULT_VOLUME_MAGNIFICATION, BGM_FADE_IN_SECONDS);

        // インゲームが書いた結果から、スコアを計算して表示する
        const GameResultData& data = GameResult::Get().Data();
        if (ui::ResultMenu* menu = m_resultPacket->GetMenu())
        {
            menu->SetResult(data, CalcScore(data.survivedSeconds, data.normalKills, data.runningKills));
        }
        return true;
    }


    void ResultScene::Update()
    {
        const GamePad* pad = ui::MenuInput::FindPad(PAD_INDEX);
        ui::ResultMenu* menu = m_resultPacket->GetMenu();

        // フェード中(インゲームから切り替わった直後など)と、「タイトルへ」が決まった後は、項目を動かさない。
        if (pad && menu && !m_isTitleRequested && !Fade::Get().IsFading())
        {
            m_menuInput.Update(*pad, g_gameTime->GetFrameDeltaTime());

            const int move = m_menuInput.GetMoveY();
            if (move != 0)
            {
                menu->MoveSelection(move);
            }

            if (pad->IsTrigger(enButtonA) || pad->IsTrigger(enButtonStart))
            {
                switch (menu->GetSelectedItem())
                {
                case ui::EnResultItem::Title:
                    m_isTitleRequested = true;
                    // 暗転と同じ時間でBGMを消す。タイトルBGMは、TitleScene::Start()でフェードインする
                    SoundManager::Get().GetBGM().FadeOutAll(FADE_DURATION);
                    break;
                case ui::EnResultItem::Quit:
                    // WM_QUITを送って、ゲームループを抜けさせる。(ウィンドウの×ボタンで閉じた時と同じ後始末になる。)
                    PostQuitMessage(0);
                    break;
                default:
                    break;
                }
            }
        }

        m_resultPacket->Update();
    }


    void ResultScene::PauseUpdate()
    {
        // 暗転している間も、UIの更新を続ける。(暗転に入るのは、「タイトルへ」を決めた時だけ)
        m_resultPacket->Update();
    }


    void ResultScene::Render(RenderContext& rc)
    {}


    void ResultScene::RenderUI(RenderContext& rc)
    {
        m_resultPacket->Render(rc);
    }


    bool ResultScene::RequestScene(uint32_t& id, float& waitTime)
    {
        if (!m_isTitleRequested)
        {
            return false;
        }

        id = NEXT_SCENE_ID;
        waitTime = FADE_DURATION;
        return true;
    }
} // namespace app
