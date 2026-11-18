/**
 * @file InGameHud.h
 * @brief インゲームのHUD
 * @details プレイヤーのHPバー・コアの耐久値バー・ハンドガンの残段数・レティクルをまとめて持つ。
 *          HPバー・コアの耐久値バー・残段数は、InGameInfoHubに通知された値を表示する。(コンストラクタで登録し、デストラクタで解除する)
 *          コアの耐久値バーは、コアの頭上の画面位置に出すので、世界座標だけは、シーンが SetCoreHpBarWorldPosition() で毎フレーム教える。
 *          例: InGameHud hud;                                          // アクターより先に作る
 *              hud.SetCoreHpBarWorldPosition(core.GetHpBarWorldPosition());  // カメラの更新後
 *              hud.Update();                                           // アクターが通知した後
 *              hud.Render(rc);                                         // RenderUI()
 */
#pragma once
#include "Source/UI/Foundation/SystemPacket.h"


namespace app
{
    namespace ui
    {
        class CoreHPBarMenu;
        class HPBarMenu;
        class HandgunAmmoMenu;
    } // namespace ui


    /**
     * @brief インゲームのHUD
     */
    class InGameHud : public Noncopyable
    {
    public:
        /**
         * @brief コンストラクタ
         * @details HUDを作り、InGameInfoHubに購読を登録する。通知された値を受け取るため、アクターより先に作ること。
         */
        InGameHud();

        /**
         * @brief デストラクタ
         * @details InGameInfoHubの購読を全て解除してから、HUDを破棄する。
         */
        ~InGameHud();


    public:
        /**
         * @brief コアの耐久値バーを出す世界座標を教える
         * @details バーはこの座標の画面位置に出る。カメラが更新された後に、毎フレーム呼ぶこと。
         * @param worldPosition バーを出す世界座標(コアの頭上)
         */
        void SetCoreHpBarWorldPosition(const Vector3& worldPosition);

        /**
         * @brief 更新処理
         * @details 全てのHUDを更新する。アクターが値を通知した後に呼ぶこと。
         */
        void Update();

        /**
         * @brief 描画処理
         * @details UIなので、シーンの RenderUI() から呼ぶ。
         * @param rc レンダーコンテキスト
         */
        void Render(RenderContext& rc);


    private:
        /** プレイヤーのHPバー。InGameInfoHubに通知されたHPを表示する */
        ui::UIPacket<ui::HPBarMenu> m_hpBarPacket;
        /** ハンドガンの残段数UI。InGameInfoHubに通知された残弾数を表示する */
        ui::UIPacket<ui::HandgunAmmoMenu> m_ammoPacket;
        /** レティクル。動かさないので、Menuは基底クラスのまま */
        ui::UIPacket<ui::MenuBase> m_reticlePacket;
        /** コアの耐久値バー。InGameInfoHubに通知された耐久値を、コアの頭上の画面位置に表示する */
        ui::UIPacket<ui::CoreHPBarMenu> m_coreHpBarPacket;
    };
} // namespace app
