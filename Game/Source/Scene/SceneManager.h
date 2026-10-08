/**
 * @file SceneManager.h
 * @brief シーンの管理をするクラス
 * @details シーンの登録・生成・破棄と、シーンの遷移(暗転 → ロード → 明転)を行う。シングルトン。
 *          シーンを追加するには、IScene を継承したクラスを作り、SceneManager のコンストラクタで AddSceneMap<T>() を呼ぶ。
 *          シーンから RequesutScene() で遷移を要求すると、SceneManager が次の順番で遷移する。
 *            Idle(通常) → FadingOut(暗転) → LoadingScene(旧シーンを破棄して新シーンを生成。IsLoaded() を待つ) → FadingIn(明転) → Idle
 *          NOTE: ポーズは未対応。
 */
#pragma once
#include <functional>
#include <map>
#include <memory>

#include "IScene.h"


namespace app
{
    /**
     * @brief シーンの管理をするクラス
     */
    class SceneManager : public Noncopyable
    {
    private:
        SceneManager();
        ~SceneManager();


    public:
        /**
         * @brief 更新
         * @details シーンの更新と、シーンの遷移の進行を行う。Fadeの更新より前に呼ぶこと。
         */
        void Update();


        /**
         * @brief 描画
         * @details ロード中は何も描かない。
         * @param rc レンダーコンテキスト
         */
        void Render(RenderContext& rc);


    public:
        /**
         * @brief インスタンスを生成する
         * @details Fadeの生成後に呼ぶこと。
         */
        static void CreateInstance();


        /**
         * @brief インスタンスを破棄する
         * @details シーンのデストラクタがFadeを使っても大丈夫なように、Fadeの破棄より前に呼ぶこと。
         */
        static void DestroyInstance();


        /**
         * @brief インスタンスを取得する
         * @return インスタンス。CreateInstance()を呼ぶ前に取得してはいけない。
         */
        static SceneManager& Get();


    private:
        /** シーン遷移の状態 */
        enum class TransitionState
        {
            /** 通常のゲームプレイ */
            Idle,
            /** FadeOut進行中(画面が暗くなっている) */
            FadingOut,
            /** 画面が暗い状態でシーンを生成・ロード中 */
            LoadingScene,
            /** FadeIn進行中(画面が明るくなっている) */
            FadingIn,
        };


        /** シーンを生成する関数 */
        using SceneFactory = std::function<std::unique_ptr<IScene>()>;


    private:
        /**
         * @brief シーンマップにシーンを追加するテンプレート関数
         * @details 追加する場合は、SceneManagerのコンストラクタで呼び出す。
         * @tparam T 追加するシーンのクラス。appScene() でIDを持っていること。
         */
        template<typename T>
        void AddSceneMap()
        {
            m_sceneMap.emplace(T::ID(), []()
                {
                    return std::unique_ptr<IScene>(std::make_unique<T>());
                });
        }


        /**
         * @brief シーンを生成して、現在のシーンにする
         * @param id 生成するシーンのID。AddSceneMap<T>() で追加されていないと生成されない。
         */
        void CreateScene(const uint32_t id);


    private:
        /** シーンマップ(シーンのIDと、シーンを生成する関数の対応) */
        std::map<uint32_t, SceneFactory> m_sceneMap;
        /** 現在のシーン */
        std::unique_ptr<IScene> m_currentScene;
        /** 次のシーンのID */
        uint32_t m_nextSceneId;
        /** 遷移の状態 */
        TransitionState m_transitionState;
        /** フェードの持続時間(秒) */
        float m_fadeDuration;


    private:
        /** 唯一のインスタンス */
        static SceneManager* m_instance;
    };
}
