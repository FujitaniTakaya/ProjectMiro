/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @details ゲームを遊ぶシーン。NOTE: 中身は未実装(シーンの骨組みだけ)。今のゲームの中身は DebugScene にある。
 */
#pragma once
#include <cstdint>

#include "IScene.h"


namespace app
{
    /**
     * @brief インゲームシーン
     */
    class InGameScene : public IScene
    {
        appScene(InGameScene);


    public:
        InGameScene();
        ~InGameScene();


        bool Start() override;
        void Update() override;
        void PauseUpdate() override;
        void Render(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;
    };
} // namespace app
