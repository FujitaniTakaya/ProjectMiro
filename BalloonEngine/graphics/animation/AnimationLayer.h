/**
 * @file AnimationLayer.h
 * @brief 1レイヤー分のアニメーションの宣言
 */
#pragma once
#include <vector>

#include "AnimationPose.h"
#include "ClipPlayer.h"
#include "CrossFader.h"


namespace nsBalloonEngine
{
    /**
     * @brief 1レイヤー分のアニメーション
     * @details ClipPlayer(クリップの再生)と CrossFader(クリップ切り替えのフェード)を1つずつ持ち、
     *          これに、レイヤーの重み(出入りのフェード)とボーンマスクを加える。
     *          ベースレイヤー以外は、Play() で重みが 1 へ、Stop() で 0 へ向かい、0 になると休止する。
     */
    class AnimationLayer
    {
    public:
        AnimationLayer();
        ~AnimationLayer() = default;


        /**
         * @brief 初期化
         * @note サンプラーの一覧とバインドポーズは、このレイヤーより長く生きていること。
         * @param samplers クリップのサンプラー一覧。添字がクリップ番号
         * @param bindPose バインドポーズ。キーの無いボーンの値として使う
         * @param isBase ベースレイヤーか。ベースは重みが常に 1 で、休止しない
         */
        void Init(const std::vector<ClipSampler>* samplers, const AnimationPose* bindPose, const bool isBase);


        /**
         * @brief クリップを再生する
         * @details 再生中のクリップと同じ番号で isReplay が false なら、何もしない(止めている最中なら再開する)。
         *          isReplay が true なら、フェードせずに最初から再生し直す。
         *          休止中のレイヤー(ベース以外)は、重みを fadeTime かけて 1 にする。
         * @param clipIndex クリップの番号
         * @param fadeTime クロスフェードの時間(秒)。0 なら切り替わる
         * @param isReplay 巻き戻して再生し直すか
         */
        void Play(const int clipIndex, const float fadeTime, const bool isReplay);


        /**
         * @brief レイヤーを止める
         * @details 重みを fadeTime かけて 0 にして、0 になったら休止する。ベースレイヤーでは何もしない。
         * @param fadeTime フェードアウトの時間(秒)。0 なら即座に止まる
         */
        void Stop(const float fadeTime);


        /**
         * @brief 更新
         * @details クリップを進め(イベントの通知とサンプリング)、クロスフェードをかけ、レイヤーの重みを進める。
         * @param deltaTime 進める時間(秒)
         * @param settings ブレンドの設定
         * @param listeners イベントの通知先
         */
        void Update(const float deltaTime, const AnimationBlendSettings& settings, const std::vector<AnimationEventListener>& listeners);


        /**
         * @brief ボーンマスクを設定する
         * @param weights ボーンごとの重み(0〜1)。ボーン数と同じ長さ
         * @param rootBone マスクの根のボーン番号。根がない(全身など)なら -1
         * @param keepRootOrientation 根のボーンが、親の向きに関わらず、このレイヤーのクリップのモデル空間の向きを保つか
         */
        void SetMask(const std::vector<float>& weights, const int rootBone, const bool keepRootOrientation);


        /**
         * @brief クリップの再生速度を設定する
         * @details クリップの時間の進み方(とイベントの発火時刻)にだけ掛ける。クロスフェードとレイヤーの重みの変化は、実時間のまま。
         * @param speed 再生速度(1.0f=通常速度)
         */
        void SetSpeed(const float speed);


        /**
         * @brief Base のボーンを、このレイヤーのクリップへ近づける設定
         * @details レイヤーが動いている間、Base のそのボーンの回転を、このレイヤーのクリップの回転へ近づける。(AnimationController が使う)
         * @param bone ボーン番号。無効にするなら -1
         */
        void SetFollowBone(const int bone);
        /**
         * @brief 近づける割合を設定する
         * @param ratio 0〜1。範囲外は丸められる
         */
        void SetFollowRatio(const float ratio);


        /**
         * @brief クリップが割り当てられているか(休止していないか)
         * @return 割り当てられているなら true
         */
        bool IsActive() const;
        /**
         * @brief クリップが再生中か
         * @details ループしないクリップは、最後まで再生すると false になる。Stop() した後も false。
         * @return 再生中なら true
         */
        bool IsPlaying() const;
        /**
         * @brief ポーズに重ねる時の、レイヤーの重みを取得
         * @param settings ブレンドの設定(イージングの有無)
         * @return 0〜1。ベースレイヤーは常に 1
         */
        float GetBlendWeight(const AnimationBlendSettings& settings) const;
        /**
         * @brief このレイヤーの出力ポーズを取得
         * @return 親ローカルのポーズ
         */
        const AnimationPose& GetPose() const;
        /**
         * @brief ボーンマスクを取得
         * @return ボーンごとの重み
         */
        const std::vector<float>& GetMask() const;
        /**
         * @brief マスクの根のボーン番号を取得
         * @return 根のボーン番号。設定されていなければ -1
         */
        int GetMaskRootBone() const;
        /**
         * @brief 根のボーンが、このレイヤーのクリップのモデル空間の向きを保つか
         * @return 保つなら true
         */
        bool KeepsRootOrientation() const;
        /**
         * @brief 近づけるボーンの番号を取得
         * @return ボーン番号。なければ -1
         */
        int GetFollowBone() const;
        /**
         * @brief 近づける割合を取得
         * @return 0〜1
         */
        float GetFollowRatio() const;


    private:
        /** 目標の重みと、そこへ向かう速さを設定する */
        void SetTargetWeight(const float target, const float duration);
        /** レイヤーを休止させる */
        void Deactivate();


    private:
        /** バインドポーズ */
        const AnimationPose* m_bindPose;
        /** クリップの再生 */
        ClipPlayer m_player;
        /** クリップ切り替えのクロスフェード */
        CrossFader m_fader;
        /** 出力ポーズ */
        AnimationPose m_pose;
        /** ボーンごとの重み */
        std::vector<float> m_mask;
        /** マスクの根のボーン番号。なければ -1 */
        int m_maskRootBone;
        /** 根のボーンが、このレイヤーのクリップのモデル空間の向きを保つか */
        bool m_keepRootOrientation;
        /** Base から近づけるボーンの番号。なければ -1 */
        int m_followBone;
        /** 近づける割合(0〜1) */
        float m_followRatio;
        /** クリップの再生速度(1が通常) */
        float m_speed;
        /** ベースレイヤーか */
        bool m_isBase;
        /** レイヤーの重み(0〜1) */
        float m_weight;
        /** レイヤーの目標の重み */
        float m_targetWeight;
        /** レイヤーの重みが変わる速さ(1/秒) */
        float m_weightSpeed;
    };
} // namespace nsBalloonEngine
