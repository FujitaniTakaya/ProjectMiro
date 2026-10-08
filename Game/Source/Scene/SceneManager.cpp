/**
 * @file SceneManager.cpp
 * @brief シーンの管理をするクラス
 */
#include "stdafx.h"

#include "SceneManager.h"

#include "Fade.h"


namespace app
{
    SceneManager* SceneManager::m_instance = nullptr;


    SceneManager::SceneManager()
        : m_sceneMap()
        , m_currentScene(nullptr)
        , m_nextSceneId(INVALID_SCENE_ID)
        , m_transitionState(TransitionState::Idle)
        , m_fadeDuration(0.0f)
    {
        // ここでシーンを追加する。
        // 例: AddSceneMap<TitleScene>();

        // 最初のシーンもここで生成する。
        // 例: CreateScene(TitleScene::ID());
    }


    SceneManager::~SceneManager()
    {}


    void SceneManager::Update()
    {
        switch (m_transitionState)
        {
        case TransitionState::Idle:
            // 通常のゲームプレイ:シーンを更新する。(シーンがまだ無いときは何もしない)
            if (!m_currentScene)
            {
                break;
            }
            m_currentScene->Update();
            // シーンの遷移要求を判定する。
            if (m_currentScene->RequestScene(m_nextSceneId, m_fadeDuration))
            {
                Fade::Get().FadeOut(m_fadeDuration);
                m_transitionState = TransitionState::FadingOut;
            }
            break;

        case TransitionState::FadingOut:
            // フェード中も、シーン固有の更新(映像再生など)を続ける。
            if (m_currentScene)
            {
                m_currentScene->PauseUpdate();
            }
            if (Fade::Get().IsFadeOutComplete())
            {
                m_currentScene.reset();
                // NOTE: 新しいシーンは、次のフレームのLoadingSceneで生成する。(暗転が画面に出てから重い処理を始める)
                m_transitionState = TransitionState::LoadingScene;
            }
            break;

        case TransitionState::LoadingScene:
            if (!m_currentScene && m_nextSceneId != INVALID_SCENE_ID)
            {
                CreateScene(m_nextSceneId);
                m_nextSceneId = INVALID_SCENE_ID;

                // シーンを生成できなかった場合(IDが登録されていない)は、暗転したまま止まらないように、シーン無しで明転する。
                // NOTE: K2_DEBUGのビルドでは、CreateScene()のK2_ASSERTで止まる。
                if (!m_currentScene)
                {
                    Fade::Get().FadeIn(m_fadeDuration);
                    m_transitionState = TransitionState::FadingIn;
                    break;
                }
            }
            if (m_currentScene)
            {
                m_currentScene->Update();

                // シーンのロードが完了するのを待つ。
                if (m_currentScene->IsLoaded())
                {
                    Fade::Get().FadeIn(m_fadeDuration);
                    m_transitionState = TransitionState::FadingIn;
                }
            }
            break;

        case TransitionState::FadingIn:
            // FadeInの完了を待つ。シーンを更新して、初期化済みのモデルなどを表示できるようにする。
            if (m_currentScene)
            {
                m_currentScene->Update();
            }
            if (!Fade::Get().IsFading())
            {
                m_fadeDuration = 0.0f;
                m_transitionState = TransitionState::Idle;
            }
            break;
        }
    }


    void SceneManager::Render(RenderContext& rc)
    {
        if (!m_currentScene)
        {
            return;
        }

        // ロード中は、シーンを描かない。
        if (m_transitionState == TransitionState::LoadingScene)
        {
            return;
        }

        m_currentScene->Render(rc);
    }


    void SceneManager::CreateInstance()
    {
        K2_ASSERT(m_instance == nullptr, "SceneManagerは既に生成されている。");
        if (m_instance == nullptr)
        {
            m_instance = new SceneManager();
        }
    }


    void SceneManager::DestroyInstance()
    {
        delete m_instance;
        m_instance = nullptr;
    }


    SceneManager& SceneManager::Get()
    {
        K2_ASSERT(m_instance != nullptr, "SceneManager::CreateInstance()を先に呼ぶこと。");
        return *m_instance;
    }


    void SceneManager::CreateScene(const uint32_t id)
    {
        const auto it = m_sceneMap.find(id);
        if (it == m_sceneMap.end())
        {
            K2_ASSERT(false, "新規シーンが追加されていません。SceneManagerのコンストラクタでAddSceneMap<T>()を呼ぶこと。\n");
            return;
        }

        m_currentScene = it->second();
        m_currentScene->Start();
    }
} // namespace app
