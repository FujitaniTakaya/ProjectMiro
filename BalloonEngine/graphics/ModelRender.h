/**
 * @file ModelRender.h
 * @brief モデル描画クラスの宣言
 */
#pragma once
#include "IRenderObject.h"
#include "ShadowRef.h"
#include "animation/AnimationController.h"
#include "math/Transform.h"


namespace nsBalloonEngine
{
    /**
     * @brief モデル描画クラス
     */
    class ModelRender : public IRenderObject
    {
    public:
        ModelRender();
        ~ModelRender() override;


    public:
        /**
         * @brief 初期化関数
         * @param tkmFilePath モデルアセットのファイルパス
         * @param isReceiveShadow 影を受けるかどうか
         * @param isCastShadow 影を落とすかどうか
         * @param upAxis モデルの上方向の軸
         * @param fxFilePath シェーダーアセットのファイルパス
         */
        void Init(
            const char* tkmFilePath,
            AnimationClip* animationClips = nullptr,
            const uint8_t animationClipNum = 0,
            const EnModelUpAxis upAxis = EnModelUpAxis::enModelUpAxisZ,
            const bool isReceiveShadow = false,
            const bool isCastShadow = false,
            const char* fxFilePath = "Assets/shader/balloon/model.fx"
        );


        /**
         * @brief 更新関数
         */
        void Update();


        /**
         * @brief 描画関数
         */
        void Draw(RenderContext& rc);


        //=======================================================================
        // トランスフォーム
        //=======================================================================
    public:
        /**
         * @brief 座標・回転・拡大を設定
         * @param position 座標
         * @param rotation 回転角度
         * @param scale 拡大率
         */
        void SetTRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        /**
         * @brief 座標・回転・拡大を設定
         * @param transform トランスフォーム
         */
        void SetTRS(const Transform& transform);
        /**
         * @brief 座標を設定
         * @param position 座標
         */
        void SetPosition(const Vector3& position);
        /**
         * @brief 回転を設定
         * @param rotation 回転角度
         */
        void SetRotation(const Quaternion& rotation);
        /**
         * @brief 拡大を設定
         * @param scale 拡大率
         */
        void SetScale(const Vector3& scale);
        /**
         * @brief トランスフォームを取得
         * @return トランスフォーム
         */
        const Transform& GetTransform() const;


        //=======================================================================
        // モデルデータ
        //=======================================================================
    public:
        /**
         * @brief モデルデータを取得
         * @return モデルデータ
         */
        Model& GetModel() const;


        /**
         * @brief ボーンを名前で探す
         * @details 武器をキャラクターの手に取り付ける時などに使う。ボーンのワールド行列は、Update() で毎フレーム更新される。
         * @note 返したポインタは、このModelRenderが生きている間だけ使える。
         * @param boneName ボーンの名前
         * @return ボーン。見つからない場合はnullptr
         */
        const Bone* FindBone(const wchar_t* boneName) const;


        //=======================================================================
        // アニメーション
        //=======================================================================
    public:
        /**
         * @brief アニメーションを再生する
         * @param clipIndex 再生するアニメーションクリップのインデックス
         * @param interpolateTime インターポレーション時間
         * @param isReplay 巻き戻して1から再生するか
         * @param start 再生し始める位置。KeepPhase なら、再生中のクリップの割合(時刻 / 長さ)の位置から始める
         */
        void PlayAnimation(const uint8_t clipIndex, const float interpolateTime = 0.0f, const bool isReplay = false, const EnAnimationStart start = EnAnimationStart::FromBeginning);


        /**
         * @brief アニメーションが再生中かどうか
         * @details ループしないクリップは、最後まで再生すると false になる。
         * @return 再生中なら true。アニメーションを持たないモデルは常に false
         */
        bool IsPlayingAnimation() const;


        /**
         * @brief アニメーション再生速度を設定する
         * @param speed 再生速度
         */
        void SetAnimationSpeed(const float speed);


        /**
         * @brief アニメーションイベントのリスナーを登録する
         * @details tka が持つイベント(発火時刻と名前)は、再生時間が発火時刻を過ぎた時に、1回だけリスナーへ通知される。
         *          通知はアニメーションの更新(Update())の中で行われる。リスナーは、ここで登録したものが全て呼ばれる。
         *          ループ・巻き戻し再生をすると、そのクリップのイベントは、また1回ずつ通知される。
         *          アニメーションを持たないモデル(Init() でクリップを渡さなかったもの)には、通知されない。
         * @note リスナーが持つポインタは、このModelRenderが生きている間は有効であること。
         * @param listener リスナー。引数はイベントを起こしたクリップの名前とイベント名
         */
        void AddAnimationEventListener(const AnimationEventListener& listener);


        /**
         * @brief レイヤーを指定してアニメーションを再生する
         * @details Upper レイヤーは、SetLayerMask() で決めたボーンの範囲だけを、Base の上に重ねる。
         *          休止中の Upper は、interpolateTime かけて重みが 1 になる。止めるには StopLayerAnimation() を呼ぶ。
         *          PlayAnimation() は、Base レイヤーでこれを呼ぶのと同じ。
         * @param layer 再生するレイヤー
         * @param clipIndex 再生するアニメーションクリップのインデックス
         * @param interpolateTime クロスフェードの時間(秒)
         * @param isReplay 巻き戻して1から再生するか(フェードしない)
         * @param start 再生し始める位置。KeepPhase なら、再生中のクリップの割合(時刻 / 長さ)の位置から始める。
         *              今のクリップと新しいクリップのどちらかがループしない時や、休止中のレイヤーは、最初から始める
         */
        void PlayLayerAnimation(const EnAnimationLayer layer, const uint8_t clipIndex, const float interpolateTime = 0.0f, const bool isReplay = false, const EnAnimationStart start = EnAnimationStart::FromBeginning);


        /**
         * @brief レイヤーのアニメーションを止める
         * @details Base は止められない(何もしない)。
         * @param layer 止めるレイヤー
         * @param fadeTime フェードアウトの時間(秒)
         */
        void StopLayerAnimation(const EnAnimationLayer layer, const float fadeTime = 0.0f);


        /**
         * @brief レイヤーのアニメーションが再生中かどうか
         * @param layer 調べるレイヤー
         * @return 再生中なら true。アニメーションを持たないモデルは常に false
         */
        bool IsPlayingLayerAnimation(const EnAnimationLayer layer) const;


        /**
         * @brief レイヤーで再生中のクリップの番号を取得する
         * @param layer 調べるレイヤー
         * @return クリップの番号。休止中のレイヤーと、アニメーションを持たないモデルは -1
         */
        int GetLayerClipIndex(const EnAnimationLayer layer) const;


        /**
         * @brief レイヤーのボーンマスクを、ボーン名で設定する
         * @details 指定したボーンとその子孫だけが、レイヤーの影響を受ける。例: 上半身は背骨の最下段(Miro は "Base HumanSpine1")。
         * @param layer 設定するレイヤー(Base には設定できない)
         * @param rootBoneName マスクの根になるボーンの名前
         * @param keepRootOrientation 根のボーンが、Base の親の向き(歩きでねじれる骨盤など)に関わらず、このレイヤーのクリップのモデル空間の向きを保つか。既定は false
         * @return 設定できたら true。アニメーションを持たないモデルや、ボーンが見つからない場合は false
         */
        bool SetLayerMask(const EnAnimationLayer layer, const wchar_t* rootBoneName, const bool keepRootOrientation = false);


        /**
         * @brief レイヤーのアニメーション再生速度を設定する
         * @details そのレイヤーのクリップの時間とイベントの発火時刻にだけ掛ける。クロスフェードとレイヤーの出入りの長さは、実時間のまま。
         *          SetAnimationSpeed() の全体の速度との積になる。例: 構え中の下半身(Base)だけを、移動速度に合わせて遅くする。
         * @param layer 設定するレイヤー
         * @param speed 再生速度(1.0f=通常速度)。0.01 未満は 0.01 になる
         */
        void SetLayerAnimationSpeed(const EnAnimationLayer layer, const float speed);


        /**
         * @brief レイヤーが動いている間、Base のボーンの回転を、そのレイヤーのクリップへ近づけるボーンを設定する
         * @details 例: 構え中、斜め歩きでねじれる骨盤(Miro は "Base HumanPelvis")を、構えたポーズの向きへ近づける。割合は SetLayerFollowRatio()。
         *          ボーンの子で、レイヤーのマスクの外のもの(脚)は、モデル空間の向きを保つ。
         * @param layer 設定するレイヤー(Base には設定できない)
         * @param boneName 近づけるボーンの名前
         * @return 設定できたら true。アニメーションを持たないモデルや、ボーンが見つからない場合は false
         */
        bool SetLayerFollowBone(const EnAnimationLayer layer, const wchar_t* boneName);


        /**
         * @brief SetLayerFollowBone() のボーンを、レイヤーのクリップへ近づける割合を設定する
         * @param layer 設定するレイヤー
         * @param ratio 0〜1。0 なら何もしない(初期値)
         */
        void SetLayerFollowRatio(const EnAnimationLayer layer, const float ratio);


    public:
        /**
         * @brief フォワードレンダリングモデルを使用するかどうかを設定する
         * @param isForwardOption フォワードレンダリングモデルを使用するか
         */
        void SetForwardOption(const bool isForwardOption);



        //=======================================================================
        // ヘルパー
        //=======================================================================
    private:
        /**
         * @brief スケルトンを初期化する
         * @param tkmFilePath モデルアセットのファイルパス
         * @param animationClips アニメーションクリップの配列
         * @param animationClipNum アニメーションクリップの数
         */
        void InitSkeleton(const char* tkmFilePath, AnimationClip* animationClips, const uint8_t animationClipNum);


        /**
         * @brief シャドウを初期化する
         * @param tkmFilePath モデルアセットのファイルパス
         * @param modelInitData モデル初期化データ
         */
        void InitShadow(const char* tkmFilePath, ModelInitData& modelInitData);


    private:
        /** モデルのボーン */
        Skeleton m_skeleton;
        /** アニメーション(レイヤーブレンド・クロスフェード) */
        AnimationController m_animationController;
        /** アニメーション再生速度 */
        float m_animationSpeed;
        /** アニメーションを渡されたか */
        bool m_isAnimated;


        /** フォワードレンダリングモデルデータ */
        Model m_forwardModel;
        /** デファードレンダリングモデルデータ */
        Model m_deferredModel;
        /** シャドウモデルデータ */
        std::array<Model, MAX_SHADOW_NUM> m_shadowModel;
        /** トランスフォーム */
        Transform m_transform;


        /** 影を受けるかどうか */
        bool m_isReceiveShadow;
        /** 影を落とすかどうか */
        bool m_isCastShadow;
        /** フォワードを使って特殊レンダリングをするかどうか */
        bool m_useForwardRendering;
    };
} // namespace nsBalloonEngine
