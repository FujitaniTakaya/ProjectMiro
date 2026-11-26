/**
 * @file ZombieStateMachine.h
 * @brief ゾンビのステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct MiroParameter;
        struct ZombieParameter;


        /**
         * @brief ゾンビのステートマシン
         * @details 待機・歩き・走り・攻撃・死亡(ZombieIState.h のゾンビ専用のステート)を登録して、入力から遷移先を決める。
         *          死亡は最優先で、HPが0になると入る。一度入ったら抜けない。
         *          待機は、入るときに慣性を消すので、入力がなくなる(コアに着く)と、滑らずにその場で止まる。
         *          攻撃は、ActionA(攻撃ボタン)を押している間に入る。動きが終わるまでは、ボタンを離しても続ける。
         *          入力は、CharacterStateMachine が持つ ControllerSlot から Update() で作る。(CharacterStateMachine を参照)
         *          歩き・走りの速度は、プレイヤーの走り速度に ZombieParameter の割合をかけた値を返す。
         *          ロックオン専用のステートができるまでは、歩き=通常時、走り=ロックオン時として扱う。(走りは、入力のダッシュで切り替わる)
         *          ゾンビ専用のステート(ロックオンなど)が要るようになったら、ここで追加する。
         * @note ZombieParameter・MiroParameter は ParamHolder が持つものの参照なので、ホットリロードで速度が変わる。
         */
        class ZombieStateMachine : public CharacterStateMachine
        {
        public:
            /**
             * @brief 歩きの移動速度を取得する
             * @return プレイヤーの走り速度 × ZombieParameter の歩きの割合
             */
            virtual float GetWalkSpeed() const override;
            /**
             * @brief 走りの移動速度を取得する
             * @return プレイヤーの走り速度 × ZombieParameter の走りの割合
             */
            virtual float GetRunSpeed() const override;
            /**
             * @brief 攻撃アニメーションの再生速度を取得する
             * @return ZombieParameter の攻撃アニメ再生速度(1.0f=通常)
             */
            float GetAttackAnimationSpeed() const;
            /**
             * @brief この個体の再生速度の倍率を取得する
             * @details 1 ± ZombieParameter の animSpeedRange。個体ごとに生まれたときに抽選した値で決まる。
             *          歩き・走りの移動速度にも同じ倍率をかけるので、足が滑らない。
             * @return 倍率(1.0f=通常)
             */
            float GetAnimationSpeedScale() const;
            /**
             * @brief この個体の開始位相を取得する
             * @details 歩き・走り・待機を始めるときに、このクリップの位置から始める。同じところから湧いたゾンビの足並みがそろわないようにする。
             * @return 位相(0〜1)。個体ごとに生まれたときに抽選した値
             */
            inline float GetPhaseOffset() const
            {
                return m_phaseOffset;
            }

            /**
             * @brief 死亡の動きが終わったか
             * @return 死亡ステートに入っていて、死亡アニメーションの再生が終わっていれば true
             */
            bool IsDeathFinished() const;

            /**
             * @brief 怯みを予約する
             * @details 次の Update() で、上半身(Upper レイヤー)に怯みのアニメーションを最初から重ねる。
             *          ステートは変わらないので、歩き・走りの移動は止まらない。攻撃中・死亡中は無視する。
             */
            inline void RequestFlinch()
            {
                m_hasFlinchRequest = true;
            }

            /**
             * @brief ステートマシンを更新する
             * @details ステートの更新のあとで、怯み(上半身)を更新する。
             */
            virtual void Update() override;


        protected:
            /**
             * @brief 攻撃に入る(続ける)かどうか
             * @return 攻撃の動きの途中、または ActionA(攻撃ボタン)を押していれば true
             */
            bool CanChangeAttackState() const;

            /**
             * @brief ステートの変更先を取得する
             * @details 優先度の高い順に判定する。死亡していれば死亡、攻撃に入る(続ける)なら攻撃、走りに切り替えられるなら走り、移動入力があれば歩き、どれにも当てはまらなければ待機。
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;


        public:
            /**
             * @brief コンストラクタ
             * @param ownerCharacter 持ち主のキャラクター
             * @param parameter Zombieのパラメーター(ParamHolderが持つもの。アプリが終わるまで使える)
             */
            ZombieStateMachine(CharacterBase* ownerCharacter, const ZombieParameter& parameter);
            virtual ~ZombieStateMachine() override = default;


        private:
            /** 怯みの上半身アニメーションを、予約と今のステートに合わせて再生・解除する */
            void UpdateFlinch();


        private:
            /** パラメーター */
            const ZombieParameter& m_parameter;
            /** プレイヤー(Miro)のパラメーター。速度の基準 */
            const MiroParameter& m_playerParameter;
            /** 怯みの予約があるか(Update() で消費する) */
            bool m_hasFlinchRequest;
            /** 再生速度の個体差の抽選値(-1〜1)。倍率はパラメーターから毎回出すので、ホットリロードに追従する */
            float m_speedVariation;
            /** 開始位相の個体差の抽選値(0〜1) */
            float m_phaseOffset;
        };
    } // namespace actor
} // namespace app
