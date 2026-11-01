/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @details インゲームの結果を表示するシーン。NOTE: 中身は未実装(シーンの骨組みだけ)。
 */
#pragma once
#include <cstdint>

#include "IScene.h"


namespace app
{
    /**
     * @brief リザルトシーン
     */
    class ResultScene : public IScene
    {
        appScene(ResultScene);


    public:
        ResultScene();
        ~ResultScene();


        bool Start() override;
        void Update() override;
        void PauseUpdate() override;
        void Render(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;
    };
} // namespace app
