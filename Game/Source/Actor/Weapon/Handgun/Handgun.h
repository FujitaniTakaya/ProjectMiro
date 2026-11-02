/**
 * @file Handgun.h
 * @brief ハンドガン
 */
#pragma once
#include "Source/Actor/ModelData.h"
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
         * @note モデルは Assets/modelData/Weapon/Glock17/Glock17.tkm(Z-up・6ボーン)。Start() でロードし、Render() で描画する。
         *       アニメーションは撃つ(スライド)・リロード(マガジン)の2つで、Fire()・Reload() が最初から再生する。
         *       手のボーンへの取り付けは AttachToBone() で行う。取り付けた後は、持ち主のUpdate()の後に、毎フレームUpdate()を呼ぶこと。
         *       ボタン入力は持ち主が受け取って Fire() を呼ぶ。(Miroの構えステートが、ZRを押した瞬間に呼ぶ)弾そのもの(当たり判定・ダメージ)は、まだ無い。
         */
        class Handgun : public WeaponBase
        {
        public:
            Handgun();
            virtual ~Handgun() override;


            /**
             * @brief 1発撃つ
             * @details 残弾を1つ減らして、撃つアニメーションを最初から再生する。リロード中と、残弾が0の時は撃てない。
             * @return 撃てたらtrue
             */
            bool Fire();

            /**
             * @brief リロードを始める
             * @details リロード時間が過ぎると、残弾が満タン(マガジンサイズ)になる。予備弾は無限。
             *          リロードのアニメーション(マガジンが動く)は最初から再生する。長さがリロード時間に収まるよう、再生速度を合わせる。
             *          リロード中や満タンの時は、何もしない。リロード時間が0以下の場合は、すぐ満タンになる。(アニメーションは再生しない)
             */
            void Reload();

            /**
             * @brief リロードできるかどうか
             * @return リロード中でも満タンでもなければtrue
             */
            inline bool CanReload() const
            {
                return !m_isReloading && m_remainingAmmo < m_magazineSize;
            }

            /**
             * @brief リロード中かどうか
             * @return リロード中ならtrue
             */
            inline bool IsReloading() const
            {
                return m_isReloading;
            }


        protected:
            /** 初期化処理。パラメーターから残弾を満タンにして、モデルをロードする */
            virtual void Start() override;
            /** 更新処理。取り付け先のボーンに追従して、リロードの時間を進めて、モデルを更新する */
            virtual void Update() override;
            /** 描画処理。モデルを描画する */
            virtual void Render(RenderContext& rc) override;


        private:
            /** リロード中か */
            bool m_isReloading;
            /** リロードが終わるまでの残り時間(秒) */
            float m_reloadTimer;
            /** アニメーションクリップ(撃つ・リロードの順) */
            AnimationClips m_animationClips;
        };
    } // namespace actor
} // namespace app
