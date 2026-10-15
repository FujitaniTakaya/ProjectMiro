/**
 * @file DebugScene.h
 * @brief デバッグ用のシーン
 * @details 開発中の動作確認に使うシーン。最初のシーンとして起動する。
 */
#pragma once
#include <cstdint>
#include <memory>

#include "IScene.h"


namespace app
{
    namespace actor
    {

        class Miro;
    }

    /**
     * @brief デバッグ用のシーン
     */
    class DebugScene : public IScene
    {
        appScene(DebugScene);


    public:
        DebugScene();
        ~DebugScene();


        bool Start() override;
        void Update() override;
        void PauseUpdate() override;
        void Render(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;


    private:
        std::unique_ptr<actor::Miro> m_miro;
    };
} // namespace app
