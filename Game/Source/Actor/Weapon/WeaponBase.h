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
         */
        class WeaponBase : public Actor
        {
        public:
            WeaponBase();
            virtual ~WeaponBase() = default;


            /**
             * @brief パラメーターから初期化する
             * @details リロード時間とマガジンサイズをパラメーターから入れ、残弾数はマガジンサイズ(満タン)にする。
             * @note パラメーターはコピーするので、Init()の後にjsonを書き換えても反映されない。
             * @param param ウェポンのパラメーター
             */
            void Init(const WeaponParameter& param);

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
            /** リロード時間(秒) */
            float m_reloadTime;
            /** マガジンサイズ(満タンの弾数) */
            uint32_t m_magazineSize;
            /** 残弾数 */
            uint32_t m_remainingAmmo;
        };
    } // namespace actor
} // namespace app
