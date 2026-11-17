/**
 * @file DebugScene.h
 * @brief デバッグ用のシーン
 * @details 開発中の動作確認に使うシーン。NOTE: 中身は空(シーンの骨組みだけ)。ゲームの中身は InGameScene にある。
 */
#pragma once
#include <cstdint>

#include "IScene.h"


namespace app
{
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
        bool IsPausable() const override;
    };
} // namespace app
