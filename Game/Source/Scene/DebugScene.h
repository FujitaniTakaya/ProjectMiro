/**
 * @file DebugScene.h
 * @brief デバッグ用のシーン
 * @details 開発中の動作確認に使うシーン。今は、タイトル画面用のスクリーンショットを撮るために、
 *          Miro・ハンドガン・弾・ステージ・空(SkyCube)だけを置いている。ゾンビ・コア・HUDは置かない。
 *          操作: 左スティック=移動、ZL=構え、ZR=撃つ、X=リロード。
 *          カメラは FreeCamera で、Miro の周りを自由に動く。右スティック=周回、RB/LB=ズーム、十字キー左右=ロール、Select=初期状態に戻す。
 *          Miro は、構えている間も、カメラの向きを向かない。(カメラを回しても、Miro の向きは変わらず、構えはその場で行う)
 *          ハンドガンと弾の発射位置の調整用だった中身は、git の履歴(DebugScene.cpp)にある。
 *          ゲームの中身は InGameScene にある。
 * @note 調整用のシーンとして残す。消さないこと。(SceneManager の最初のシーンを DebugScene::ID() にすると、ここから始まる)
 */
#pragma once
#include <cstdint>
#include <memory>

#include "IScene.h"
#include "Source/Actor/Stage/StageBoundary.h"


namespace app
{
    namespace actor
    {
        class Handgun;
        class Miro;
        class StageField;
    } // namespace actor

    namespace camera
    {
        class FreeCamera;
    } // namespace camera


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


    private:
        /**
         * @brief 1発撃つ
         * @details ハンドガンが撃てたら、銃口から、銃の向きそのままに弾を出す。(カメラの視線が指す点へは寄せない)
         *          InGameScene::FireHandgun から、音・HUD・カメラの反動・視線への狙い寄せを除いたもの。
         * @return 撃てたらtrue。弾切れ・リロード中はfalse
         */
        bool FireHandgun();


    private:
        /** ステージ(モデル) */
        std::unique_ptr<actor::StageField> m_stageField;
        /** 操作するMiro */
        std::unique_ptr<actor::Miro> m_miro;
        /** Miroの手に持たせるハンドガン */
        std::unique_ptr<actor::Handgun> m_handgun;
        /** Miroの周りを自由に動くカメラ */
        std::unique_ptr<camera::FreeCamera> m_freeCamera;
        /** ステージの当たり判定(モデルの形そのまま)。Miroが床の上を歩ける */
        PhysicsStaticObject m_bgPhysicsStaticObject;
        /** ステージの外周の見えない壁。Miroが外へ落ちない */
        actor::StageBoundary m_stageBoundary;
    };
} // namespace app
