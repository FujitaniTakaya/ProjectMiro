/**
 * @file WeaponBase.h
 * @brief ウェポンの基底クラス
 */
#pragma once
#include <cstdint>

#include "Source/Actor/Actor.h"


namespace app
{
    namespace actor
    {
        struct WeaponParameter;


        /**
         * @brief ウェポンの基底クラス
         * @details コンストラクタでは全てのメンバーを0にする。値はInit()でパラメーター(json)から入れる。
         *          Start()・Update()・Render()はActorの純粋仮想関数のまま実装していないので、派生クラスで実装すること。
         *          キャラクターの手のボーンに取り付けられる。AttachToBone() で取り付けると、派生クラスのUpdate()が
         *          UpdateAttachedTransform() を呼ぶたびに、武器のトランスフォームがボーンに追従する。
         */
        class WeaponBase : public Actor
        {
        public:
            WeaponBase();
            virtual ~WeaponBase() = default;


            /**
             * @brief パラメーターから初期化する
             * @details リロード時間とマガジンサイズをパラメーターから入れ、残弾数はマガジンサイズ(満タン)にする。
             *          手に持たせる時の握りのオフセット(gripPosition・gripRotation)は、パラメーターの参照を持って毎フレーム読む。
             * @note リロード時間とマガジンサイズはコピーするので、Init()の後にjsonを書き換えても反映されない。
             *       握りのオフセットだけは、ホットリロードがすぐ反映される。そのため、パラメーターはParamHolderのもの(アプリが終わるまで使える)を渡すこと。
             * @param param ウェポンのパラメーター
             */
            void Init(const WeaponParameter& param);

            /**
             * @brief キャラクターのボーンに取り付ける
             * @details 取り付けている間、UpdateAttachedTransform() が武器のトランスフォームを、
             *          ボーンのワールド行列に握りのオフセットを掛けたもので上書きする。ボーンの座標は渡さず、ボーンそのものを持つ。
             *          付け替える時は、もう一度呼ぶ。
             * @note ボーンは持ち主のModelRenderのもの。持ち主より先に武器を捨てるか、持ち主が無くなる前にDetach()すること。
             *       持ち主のUpdate()の後に、この武器のUpdate()を呼ぶこと。(ボーンのワールド行列が、そのフレームの値になる)
             * @param bone 取り付け先のボーン(ModelRender::FindBone() で取る)。nullptrの場合は取り外す
             */
            void AttachToBone(const Bone* bone);

            /**
             * @brief 取り外す
             * @details 武器のトランスフォームは、取り外した時のまま残る。
             */
            void Detach();

            /**
             * @brief ボーンに取り付けているかどうか
             * @return 取り付けていればtrue
             */
            inline bool IsAttached() const
            {
                return m_attachBone != nullptr;
            }

            /**
             * @brief リロード時間を取得
             * @return リロード時間(秒)
             */
            inline float GetReloadTime() const
            {
                return m_reloadTime;
            }
            /**
             * @brief マガジンサイズを取得
             * @return マガジンサイズ(満タンの弾数)
             */
            inline uint32_t GetMagazineSize() const
            {
                return m_magazineSize;
            }
            /**
             * @brief 残弾数を取得
             * @return 残弾数
             */
            inline uint32_t GetRemainingAmmo() const
            {
                return m_remainingAmmo;
            }


        protected:
            /**
             * @brief 取り付け先のボーンに追従させる
             * @details 取り付けている時だけ、トランスフォームの座標・回転・拡大を上書きする。取り付けていない時は何もしない。
             *          上書きするのは、ModelRenderに渡す前のトランスフォーム。そのため、派生クラスのUpdate()の先頭で呼ぶこと。
             *          武器のモデルの上方向の補正(Z-upなど)は、ModelRender側で掛かるので、ここで打ち消してある。
             * @note 持ち主の拡大率は、武器にもそのまま掛かる。(一様な拡大だけに対応)
             */
            void UpdateAttachedTransform();


        protected:
            /** リロード時間(秒) */
            float m_reloadTime;
            /** マガジンサイズ(満タンの弾数) */
            uint32_t m_magazineSize;
            /** 残弾数 */
            uint32_t m_remainingAmmo;


        private:
            /** パラメーター(ParamHolderが持つもの)。握りのオフセットを毎フレーム読むために持つ。Init()の前はnullptr */
            const WeaponParameter* m_parameter;
            /** 取り付け先のボーン。取り付けていない時はnullptr */
            const Bone* m_attachBone;
        };
    } // namespace actor
} // namespace app
