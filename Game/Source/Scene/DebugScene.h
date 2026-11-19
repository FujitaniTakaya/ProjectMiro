/**
 * @file DebugScene.h
 * @brief デバッグ用のシーン
 * @details 開発中の動作確認に使うシーン。今は、ハンドガンと弾の発射位置・向きの調整用。
 *          ハンドガンを空中に固定して置き、撃つと、Handgun.json の muzzle* の位置から弾が出る。(jsonはホットリロードで反映される)
 *          操作: ZR=撃つ、X=リロード、右スティック=カメラを回す。ImGuiの窓からも操作できる。
 *          ゲームの中身は InGameScene にある。
 * @note 調整用のシーンとして残す。消さないこと。(SceneManager の最初のシーンを DebugScene::ID() にすると、ここから始まる)
 */
#pragma once
#include <cstdint>
#include <memory>

#include "IScene.h"


namespace app
{
    namespace actor
    {
        class Handgun;
    } // namespace actor


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
         * @details ハンドガンが撃てたら、銃口(WeaponBase::CalcMuzzle)の位置と向きで、弾を1発出す。
         */
        void Fire();

        /**
         * @brief カメラを、ハンドガンの周りを回る位置に置く
         */
        void UpdateCamera();


    private:
        /** 調整するハンドガン。空中の固定の位置に置く(手には持たせない) */
        std::unique_ptr<actor::Handgun> m_handgun;
        /** 弾の発射スピード(cm/秒)。ImGuiで変える */
        float m_bulletSpeed;
        /** カメラの左右の角度(度)。ハンドガンの周りを回る */
        float m_cameraYawDeg;
        /** カメラの上下の角度(度) */
        float m_cameraPitchDeg;
        /** カメラからハンドガンまでの距離(cm) */
        float m_cameraDistance;
    };
} // namespace app
