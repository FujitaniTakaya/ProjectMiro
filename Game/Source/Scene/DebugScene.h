/**
 * @file DebugScene.h
 * @brief デバッグ用のシーン
 * @details 開発中の動作確認に使うシーン。最初のシーンとして起動する。
 */
#pragma once
#include <cstdint>
#include <memory>

#include "IScene.h"
#include "Source/UI/SystemPacket.h"


namespace app
{
    namespace actor
    {

        class Miro;
        class Handgun;
        class StageField;
        class Core;
    } // namespace actor
    namespace camera
    {
        class PlayerCamera;
    }
    namespace ui
    {
        class HPBarMenu;
        class HandgunAmmoMenu;
    } // namespace ui

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
        void RenderUI(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;


    private:
        std::unique_ptr<actor::Miro> m_miro;
        std::unique_ptr<camera::PlayerCamera> m_playerCamera;
        /** プレイヤーのHPバー。InGameInfoHubに通知されたHPを表示する */
        ui::UIPacket<ui::HPBarMenu> m_hpBarPacket;
        /** ハンドガン。弾が減ったら、InGameInfoHubに残弾数を通知する(手に持たせるまでは、このシーンが持つ) */
        std::unique_ptr<actor::Handgun> m_handgun;
        /** ハンドガンの残段数UI。InGameInfoHubに通知された残弾数を表示する */
        ui::UIPacket<ui::HandgunAmmoMenu> m_ammoPacket;
        /** レティクル。動かさないので、Menuは基底クラスのまま */
        ui::UIPacket<ui::MenuBase> m_reticlePacket;
        /** ステージのフィールド(背景のstage.tkm) */
        std::unique_ptr<actor::StageField> m_stageField;
        /** ステージのコア(宝石のモデル)。当たり判定は無い */
        std::unique_ptr<actor::Core> m_core;
        /** 背景の当たり判定。m_stageFieldのモデルの形そのままのメッシュコライダー(キャラクターコントローラーが床や壁として扱う) */
        PhysicsStaticObject m_bgPhysicsStaticObject;
    };
} // namespace app
