/**
 * @file Handgun.h
 * @brief ハンドガン
 */
#pragma once
#include "Source/Actor/Weapon/WeaponBase.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ハンドガン
         * @details デフォルトの武器。予備弾は無限なので、マガジンの残弾が減るだけで、リロードすれば必ず満タンに戻る。
         *          値(リロード時間・マガジンサイズ)は Assets/parameter/actor/Handgun.json(EnParamID::Handgun)から、Start() で WeaponBase::Init() に入れる。
         *          Start() を一度呼んでから、毎フレーム Update() を呼ぶ。
         * @note モデルはまだ無いので、Render() は何も描画しない。
         *       ボタン入力・弾の発射・手への取り付けも、まだ無い。
         */
        class Handgun : public WeaponBase
        {
        public:
            Handgun();
            virtual ~Handgun() override;


            /**
             * @brief 1発撃つ
             * @details 残弾を1つ減らす。リロード中と、残弾が0の時は撃てない。
             * @return 撃てたらtrue
             */
            bool Fire();

            /**
             * @brief リロードを始める
             * @details リロード時間が過ぎると、残弾が満タン(マガジンサイズ)になる。予備弾は無限。
             *          リロード中や満タンの時は、何もしない。リロード時間が0以下の場合は、すぐ満タンになる。
             */
            void Reload();

            /**
             * @brief リロード中かどうか
             * @return リロード中ならtrue
             */
            inline bool IsReloading() const
            {
                return m_isReloading;
            }


        protected:
            /** 初期化処理。パラメーターから残弾を満タンにする */
            virtual void Start() override;
            /** 更新処理。リロードの時間を進める */
            virtual void Update() override;
            /** 描画処理。モデルが無いので何も描画しない */
            virtual void Render(RenderContext& rc) override;


        private:
            /** リロード中か */
            bool m_isReloading;
            /** リロードが終わるまでの残り時間(秒) */
            float m_reloadTimer;
        };
    } // namespace actor
} // namespace app
