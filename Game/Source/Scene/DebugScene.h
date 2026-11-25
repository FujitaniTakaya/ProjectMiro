/**
 * @file DebugScene.h
 * @brief デバッグ用のシーン
 * @details 開発中の動作確認に使うシーン。今は、タイトル画面用のスクリーンショットを撮るために、
 *          ステージ・空(SkyCube)・ゾンビだけを置いている。コア・HUDは置かない。
 *          Miro・ハンドガン・弾の発射は、一時的にコメントアウトしている。(スクリーンショットにMiroが要らないため)
 *          ゾンビは、普通のゾンビ3体(その場で、何秒かおきに攻撃アニメーションを出す)と、走るゾンビ数体(その場へ走ってきて、着いたら攻撃する)。
 *          カメラは FreeCamera で、ゾンビの集まりの周りを自由に動く。右スティック=周回、RB/LB=ズーム、十字キー左右=ロール、Select=初期状態に戻す。
 *          ハンドガンと弾の発射位置の調整用だった中身は、git の履歴(DebugScene.cpp)にある。
 *          ゲームの中身は InGameScene にある。
 * @note 調整用のシーンとして残す。消さないこと。(SceneManager の最初のシーンを DebugScene::ID() にすると、ここから始まる)
 */
#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "IScene.h"
#include "Source/Actor/Stage/StageBoundary.h"


namespace app
{
    namespace actor
    {
        // class Handgun;
        // class Miro;
        class StageField;
        class ZombieBase;
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
        // 一時的にコメントアウト(Miroを使わない)
        // /**
        //  * @brief 1発撃つ
        //  * @details ハンドガンが撃てたら、銃口から、銃の向きそのままに弾を出す。(カメラの視線が指す点へは寄せない)
        //  *          InGameScene::FireHandgun から、音・HUD・カメラの反動・視線への狙い寄せを除いたもの。
        //  * @return 撃てたらtrue。弾切れ・リロード中はfalse
        //  */
        // bool FireHandgun();

        /**
         * @brief ゾンビを置く
         * @details 普通のゾンビ3体(一か所にまとめて、何秒かおきに攻撃)と、走るゾンビ数体(まとまりの周りから、まとまりへ走る)を作る。
         *          ステージの当たり判定ができた後に呼ぶこと。(足元が床に着く)
         */
        void SpawnZombies();


    private:
        /** ステージ(モデル) */
        std::unique_ptr<actor::StageField> m_stageField;
        // 一時的にコメントアウト(Miroを使わない)
        // /** 操作するMiro */
        // std::unique_ptr<actor::Miro> m_miro;
        // /** Miroの手に持たせるハンドガン */
        // std::unique_ptr<actor::Handgun> m_handgun;
        /** ゾンビ(普通のゾンビ・走るゾンビ)。Start() で作る */
        std::vector<std::unique_ptr<actor::ZombieBase>> m_zombies;
        /** ゾンビの集まりの周りを自由に動くカメラ */
        std::unique_ptr<camera::FreeCamera> m_freeCamera;
        /** ステージの当たり判定(モデルの形そのまま)。Miroが床の上を歩ける */
        PhysicsStaticObject m_bgPhysicsStaticObject;
        /** ステージの外周の見えない壁。Miroが外へ落ちない */
        actor::StageBoundary m_stageBoundary;
    };
} // namespace app
