/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @details ゲームを遊ぶシーン。ステージ・コア・Miro・ゾンビ・ハンドガン・カメラ・HUDを組み立てる。タイトルシーンでスタートを押すと進む。
 */
#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "InGameHud.h"
#include "Source/Actor/Stage/StageBoundary.h"
#include "Source/Core/Timer/Timer.h"
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
        class GameSetMenu;
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

        /**
         * @brief コアの耐久値かMiroのHPが0になったかを調べて、ゲームセットにする
         * @details 0になったら m_isGameOver を立て、結果(守衛時間・倒した数)を GameResult に書き、Miroからパッドのコントローラーを外し、「Finish」を出す。一度なったら、リザルトへ進むまでの待ち時間を進める。
         *          待ち時間が過ぎると、RequestScene() がリザルトシーンへの遷移を要求する。
         */
        void UpdateGameOver();

        /**
         * @brief ハンドガンを1発撃つ
         * @details ハンドガンが撃てたら、銃口(WeaponBase::CalcMuzzle)の位置から、レティクル(カメラの視線)が指す点へ向けて、弾を1発出す。
         *          胸から銃口までの間に壁・ゾンビがある時は、弾を出さず、その場で着弾にする。
         *          弾切れ・リロード中は撃てない。弾は BulletManager が持つ。
         * @return 撃てたらtrue(Miroの武器ハンドラの fire に返す)
         */
        bool FireHandgun();

        /**
         * @brief 構えている時、レティクルが生きているゾンビを指しているか
         * @details 弾が狙う点と同じ視線(actor::SweepAimRay)を伸ばして、最初に当たった物がゾンビの体かを調べる。
         *          構えを保持していない間は、撃てないので false。死んだゾンビ(死亡アニメーション中)も false。
         * @return 指しているなら true
         */
        bool IsAimingAtZombie() const;


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
        /** ステージの外周の見えない壁。ステージの外へ出られなくする(描画はしない) */
        actor::StageBoundary m_stageBoundary;
        /** 開始前のカウントダウンの時間。0になるまでは、Miroを操作できず、ゾンビも湧かず、ポーズもできない */
        StartCountdown m_countdown;
        /** 前のフレームに出していたカウントダウンの数字。数字が切り替わった時に、SEを鳴らすために覚えておく。まだ出していなければ0 */
        int32_t m_lastCountdownNumber;
        /** 経過時間(秒)。カウントダウンが終わってから進み、ゲームセットで止まる */
        float m_elapsedTime;
        /** ゲームセットになったか。立つと、経過時間・ゾンビの更新が止まり、Miroは操作できなくなる */
        bool m_isGameOver;
        /** 「Finish」を出してから、リザルトへ進むまでの待ち時間のタイマー。ゲームセットになってから進み、時間切れになったらリザルトへ遷移する */
        Timer m_finishTimer;
        /** カウントダウンの数字のUI */
        ui::UIPacket<ui::CountdownMenu> m_countdownPacket;
        /** 「Finish」の文字のUI。ゲームセットになった時に出す */
        ui::UIPacket<ui::GameSetMenu> m_gameSetPacket;
        /** カウントダウン中に、Miroから外しておいたパッドのコントローラー。0になったら、Miroへ付け直す。ゲームセットの時にも、外したものをここへ入れる */
        std::unique_ptr<IController> m_playerController;
        /** エイムアシストの対象(生きているゾンビの胸の座標)。構え中に毎フレーム集める。メモリの確保を繰り返さないよう、使い回す */
        std::vector<Vector3> m_aimTargets;
    };
} // namespace app
