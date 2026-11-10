/**
 * @file Core.h
 * @brief ステージのコア
 */
#pragma once
#include "Source/Actor/Actor.h"
#include "physics/PhysicsGhostObject.h"


namespace app
{
    namespace actor
    {
        struct CoreParameter;


        /**
         * @brief ステージのコア
         * @details 宝石のモデル(Gemme.tkm)を描画する。
         *          地面から少し浮いた位置で、X軸まわりに少し傾いたまま、自身の軸で常に回転する。
         *          SetPosition() には、コアの真下の地面の座標を渡す(浮かせる高さは Core 側で足す)。
         *          SetRotation() で設定した回転は、毎フレームの回転で上書きされる。
         *          耐久値(HP)を持つ。最大耐久値はCoreParameter(Core.json)で、現在の耐久値は最大耐久値で始まる。
         *          耐久値は、InGameInfoHub::NotifyCoreHp() でUIに伝える。(値が変わった時だけ伝わる)
         *          宝石の中心に、箱の当たり判定(PhysicsGhostObject)を持つ。大きさはCoreParameterのhitBoxWidth・hitBoxHeight。
         *          ゾンビの攻撃判定など、外から重なった判定が自分の箱かどうかは IsHitBox() で調べ、重なったら TakeDamage() を呼んでもらう。
         *          Start() を一度呼んでから、毎フレーム Update() と Render() を呼ぶ。
         * @note モデルは Assets/modelData/stage/core/Gemme.tkm(Z-up・アニメーション無し)。
         *       モデルはメートル単位で書き出されている(高さ1.32)。ワールドはcm単位なので、拡大率の初期値は100倍にしてあり、さらに1.6倍している。
         *       モデルの原点は中心から少しずれているので、宝石の中心を軸にして回す。
         *       当たり判定の箱は回転しない(軸に合わせたまま)。大きさはStart()で決まり、ホットリロードしても次に起動するまで反映されない。
         *       箱はすり抜けるので、キャラクターの移動は邪魔しない。
         *       最大耐久値はホットリロードで変わる。現在の耐久値は丸められないので、最大耐久値を下げると、現在の耐久値が最大耐久値を超えることがある。
         */
        class Core : public Actor
        {
        public:
            Core();
            virtual ~Core() override;


        public:
            /**
             * @brief 現在の耐久値を取得
             * @return 現在の耐久値
             */
            inline float GetHp() const
            {
                return m_hp;
            }

            /**
             * @brief 最大耐久値を取得
             * @return 最大耐久値(CoreParameterのmaxHp)
             */
            float GetMaxHp() const;

            /**
             * @brief ダメージを受ける
             * @param damage ダメージ量。0以下の場合は何もしない
             */
            void TakeDamage(const float damage);

            /**
             * @brief 回復する
             * @param amount 回復量。0以下の場合は何もしない
             */
            void Heal(const float amount);

            /**
             * @brief 耐久値バーを出す基準点のワールド座標を取得
             * @details 宝石の中心から、CoreParameterのhpBarOffsetYだけ上。UIで画面の位置に変換して使う。
             * @return ワールド座標
             */
            Vector3 GetHpBarWorldPosition() const;

            /**
             * @brief 渡された判定が、コアの当たり判定の箱かどうかを調べる
             * @details 攻撃判定に重なった物のうち、コアのものだけを見分けるのに使う。
             * @param object 物理ワールドで重なった判定
             * @return コアの当たり判定の箱なら true
             */
            bool IsHitBox(const btCollisionObject& object) const;


        protected:
            /** 初期化処理。モデルをロードして、ワールド行列を確定させる */
            virtual void Start() override;
            /** 更新処理。回転を進めて、モデルを更新する */
            virtual void Update() override;
            /** 描画処理。モデルを描画する */
            virtual void Render(RenderContext& rc) override;


        private:
            /** 現在の回転と座標をモデルへ反映して、ワールド行列を更新する */
            void UpdateModel();

            /**
             * @brief 宝石の中心のワールド座標を求める
             * @return 宝石の中心。地面の座標(SetPosition())から、浮かせた高さと宝石の中心までの高さを足した点
             */
            Vector3 CalcCenter() const;

            /**
             * @brief 現在の耐久値を設定
             * @param hp 現在の耐久値。0〜最大耐久値に丸められる
             */
            void SetHp(const float hp);


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const CoreParameter& m_parameter;
            /** 現在の自転の角度(度)。0以上360未満 */
            float m_yawDeg;
            /** 現在の耐久値 */
            float m_hp;
            /** 当たり判定の箱(宝石の中心に置く。物理ワールドに登録されるのは Start() から) */
            PhysicsGhostObject m_hitBox;
        };
    } // namespace actor
} // namespace app
