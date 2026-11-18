/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @details ゲームを遊ぶシーン。ステージ・コア・Miro・ゾンビ・ハンドガン・カメラ・HUDを組み立てる。タイトルシーンでスタートを押すと進む。
 */
#pragma once
#include <cstdint>
#include <memory>

#include "InGameHud.h"
#include "Source/Scene/IScene.h"
#include "Source/UI/Foundation/SystemPacket.h"
#include "StartCountdown.h"


namespace app
{
    class IController;


    namespace actor
    {

        class Miro;
        class ZombieManager;
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
        class CountdownMenu;
    } // namespace ui

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
        void RenderUI(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;
        bool IsPausable() const override;


    private:
        /**
         * @brief 開始前のカウントダウンを進める
         * @details 数字の表示と、0になった時の操作の開始(Miroへパッドのコントローラーを付け直す)を行う。
         *          0になった後は、何もしない。
         */
        void UpdateCountdown();


    private:
        /**
         * HUD(HPバー・コアの耐久値バー・残段数・レティクル)。InGameInfoHubへの登録を持つので、アクターより先に作る。
         * 宣言順の先頭に置くことで、アクターより後に破棄される(アクターの破棄中も、登録先が生きている)
         */
        InGameHud m_hud;
        std::unique_ptr<actor::Miro> m_miro;
        std::unique_ptr<camera::PlayerCamera> m_playerCamera;
        /** ハンドガン。弾が減ったら、InGameInfoHubに残弾数を通知する(手に持たせるまでは、このシーンが持つ) */
        std::unique_ptr<actor::Handgun> m_handgun;
        /** ステージのフィールド(背景のstage.tkm) */
        std::unique_ptr<actor::StageField> m_stageField;
        /** ステージのコア(宝石のモデル)。当たり判定は無い */
        std::unique_ptr<actor::Core> m_core;
        /** ゾンビのマネージャー。4つのゲートからゾンビを湧かせて、コアへ向かわせる。ゾンビの生成・破棄・保持も行う。コアの位置の関数を持つので、m_core より後ろに置く(先に破棄される) */
        std::unique_ptr<actor::ZombieManager> m_zombieManager;
        /** 背景の当たり判定。m_stageFieldのモデルの形そのままのメッシュコライダー(キャラクターコントローラーが床や壁として扱う) */
        PhysicsStaticObject m_bgPhysicsStaticObject;
        /** 開始前のカウントダウンの時間。0になるまでは、Miroを操作できず、ゾンビも湧かず、ポーズもできない */
        StartCountdown m_countdown;
        /** カウントダウンの数字のUI */
        ui::UIPacket<ui::CountdownMenu> m_countdownPacket;
        /** カウントダウン中に、Miroから外しておいたパッドのコントローラー。0になったら、Miroへ付け直す */
        std::unique_ptr<IController> m_playerController;
    };
} // namespace app
