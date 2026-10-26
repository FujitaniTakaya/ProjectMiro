/**
 * @file CoreHPBarMenu.h
 * @brief コアの耐久値バーを動かすMenu
 * @details jsonは HPBarMenu と同じ名前のUIGauge(GreyGauge / RedGauge / GreenGauge)を並べる。(Assets/parameter/ui/CoreHPBar.json)
 *          バーは、キャンバスの原点が中心になるように置く。キャンバスを、3D空間の点を画面に変換した位置へ動かして使う。
 */
#pragma once
#include "Source/UI/PlayerHP/HPBarMenu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief コアの耐久値バーを動かすMenu
         * @details バーの見た目と動きは HPBarMenu と同じ。
         *          UpdateScreenPosition() で3D空間の点を受け取って、その点がカメラに映っている間だけ、画面上の位置にバーを描画する。
         *          映っていない間も、バーの動き(赤のバーが追いつく動き)は進める。
         *          毎フレーム、カメラを更新した後に UpdateScreenPosition() → Update() の順に呼ぶ。
         */
        class CoreHPBarMenu : public HPBarMenu
        {
        public:
            CoreHPBarMenu();
            ~CoreHPBarMenu() override;


        public:
            /**
             * @brief 更新
             * @details キャンバスを画面上の位置へ動かしてから、HPBarMenu の更新をする。
             */
            void Update() override;

            /**
             * @brief 描画
             * @details 3D空間の点がカメラに映っていない時は、何も描画しない。
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc) override;


        public:
            /**
             * @brief バーを出す3D空間の点を設定
             * @details g_camera3D で画面上の位置に変換する。カメラの視錐台の外(後ろ・左右・上下・手前と奥のクリップ面の外)にある点は、映っていないとして描画しない。
             * @param worldPosition バーの基準点のワールド座標
             */
            void UpdateScreenPosition(const Vector3& worldPosition);

            /**
             * @brief カメラに映っているかどうか
             * @return 最後に UpdateScreenPosition() で渡した点が映っていればtrue
             */
            bool IsVisible() const
            {
                return m_isVisible;
            }

            /**
             * @brief 画面上の位置を取得
             * @return 画面の中心を原点、上をYの正とした位置(ピクセル)。映っていない時は、最後に映っていた位置
             */
            const Vector2& GetScreenPosition() const
            {
                return m_screenPosition;
            }


        private:
            /** 画面上の位置(画面の中心が原点、上がYの正) */
            Vector2 m_screenPosition;
            /** カメラに映っているか */
            bool m_isVisible;
        };
    } // namespace ui
} // namespace app
