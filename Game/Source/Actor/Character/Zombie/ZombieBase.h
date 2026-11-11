/**
 * @file ZombieBase.h
 * @brief ゾンビの基底クラス
 */
#pragma once
#include <memory>

#include "Source/Actor/Character/CharacterBase.h"
#include "ZombieAttackHandler.h"
#include "physics/PhysicsGhostObject.h"


namespace app
{
    /** 前方宣言 */
    class ControllerSlot;


    namespace actor
    {
        /** 前方宣言 */
        struct ZombieParameter;
        class ZombieStateMachine;


        /**
         * @brief ゾンビの基底クラス
         * @details 全種類のゾンビの元になる。次の3つを持つ。
         *          - ZombieStateMachine : 入力から状態(待機・歩き・走り)を決める。入力を作る ControllerSlot もここが持つ
         *          - ZombieParameter : ParamHolderから取り出した参照。ホットリロードで値が変わる(最大HPもここ)
         *          - 累積アンガー値 : 現在のHPと同じように、ゾンビが実行中に持つ値。Start() で初期アンガー値から始まる
         *          ゾンビの種類は、この派生クラスで作る。(例: NormalZombie)種類ごとのパラメーター(EnParamID と同じ ZombieParameter のjson)と
         *          モデルデータを、コンストラクタで渡すこと。モデルデータのアニメーションは、EnZombieAnimationClipID の順に並べる。
         *          Update() の順は、ステートマシン(入力の作成 → ステートの更新) → モデルの更新。
         *          Miroと違い、Start() ではコントローラーを付けない。付けるまでは無入力(待機)になる。
         *          AIController を付けるなら、GetControllerSlot() から付ける。目標の設定も、ゾンビを置く側が行う。
         *          攻撃のダメージは、攻撃アニメ(Zombie_Attack.tka)のイベントで決まる。
         *          - AttackBegin : 右手(hand_r)に攻撃判定の箱を出す。大きさは ZombieParameter の attackBoxSize
         *          - AttackEnd   : 攻撃判定の箱を消す
         *          箱が出ている間、毎フレーム、重なっている物を SetAttackHandler() の処理に渡す。
         *          処理が当たりとして扱ったら(true を返したら)、その振りではもう渡さない。(1振りで1回だけダメージが入る)
         * @note アンガー値の加算・本格ロックオンへの移行・周囲への伝播は、まだ無い。
         *       攻撃判定は右手だけ。(攻撃アニメで振り下ろされるのが右手のため)
         *       初期座標は、Start() の前に SetPosition() で設定しておくこと。(キャラクターコントローラーが現在の座標に作られる)
         */
        class ZombieBase : public CharacterBase
        {
        public:
            /**
             * @brief コントローラーの入れ物を取得
             * @details ステートマシンが持つ入れ物を返す。Attach() / Detach() でコントローラーを付け替えられる。
             * @return コントローラーの入れ物
             */
            ControllerSlot& GetControllerSlot();

            /**
             * @brief パラメーターを取得
             * @details このゾンビの種類のパラメーター。攻撃力・アンガーの値などを読むときに使う。
             * @return ZombieParameter の参照
             */
            inline const ZombieParameter& GetParameter() const
            {
                return m_parameter;
            }

            /**
             * @brief 最大HPを取得
             * @return ZombieParameter の最大HP
             */
            virtual float GetMaxHp() const override;

            /**
             * @brief 累積アンガー値を取得
             * @return 累積アンガー値
             */
            inline float GetAnger() const
            {
                return m_anger;
            }
            /**
             * @brief 累積アンガー値を設定
             * @param anger 累積アンガー値。0未満は0に丸められる
             */
            void SetAnger(const float anger);

            /**
             * @brief 攻撃判定が何かに重なった時の処理を設定
             * @details Start() の前でも後でもよい。次の攻撃の振りから使われる。
             * @param handler 重なった時の処理。空なら何にも当たらない
             */
            void SetAttackHandler(const ZombieAttackHandler& handler);


        public:
            /** デストラクタ。ZombieManager が ZombieBase のポインタで持って破棄するので、公開している(直接作れないのはコンストラクタが protected なため) */
            virtual ~ZombieBase() override;


        protected:
            /**
             * @brief コンストラクタ
             * @param parameter このゾンビの種類のパラメーター(ParamHolderが持つもの。アプリが終わるまで使える)
             * @param modelData このゾンビの種類のモデルデータ(アプリが終わるまで使えるもの。静的な定数を渡す)
             */
            ZombieBase(const ZombieParameter& parameter, const ModelData& modelData);


        protected:
            /** 初期化処理 */
            virtual void Start() override;
            /** 更新処理 */
            virtual void Update() override;
            /** 描画処理 */
            virtual void Render(RenderContext& rc) override;


        private:
            /**
             * @brief アニメーションイベントを受け取る
             * @param eventName イベント名(AttackBegin / AttackEnd 以外は無視する)
             */
            void OnAnimationEvent(const wchar_t* eventName);

            /** 攻撃判定の箱を右手に合わせて、重なった物を攻撃の処理に渡す(箱が出ている間だけ) */
            void UpdateAttackHitBox();


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const ZombieParameter& m_parameter;
            /** モデルデータ(アプリが終わるまで使えるもの) */
            const ModelData& m_modelData;
            /** ステートマシン */
            std::unique_ptr<ZombieStateMachine> m_stateMachine;
            /** 累積アンガー値 */
            float m_anger;
            /** 攻撃判定の箱(AttackBegin から AttackEnd の間だけ、物理ワールドに登録される) */
            PhysicsGhostObject m_attackHitBox;
            /** 攻撃判定が重なった時の処理 */
            ZombieAttackHandler m_attackHandler;
            /** 攻撃判定を付ける右手のボーン(Start() の後に取る。見つからなければ nullptr で、攻撃判定は出ない) */
            const Bone* m_attackBone;
            /** 攻撃判定の箱が出ているか */
            bool m_isAttackActive;
            /** 今の振りで、もう当たったか */
            bool m_hasAttackHit;
        };
    } // namespace actor
} // namespace app
