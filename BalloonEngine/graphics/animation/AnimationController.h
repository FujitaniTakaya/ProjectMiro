/**
 * @file AnimationController.h
 * @brief レイヤーブレンドを行うアニメーション制御クラスの宣言
 */
#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "AnimationLayer.h"


namespace nsBalloonEngine
{
    /**
     * @brief アニメーションのレイヤー
     */
    enum class EnAnimationLayer : uint8_t
    {
        /** 全身のベース。常に再生され、止められない */
        Base,
        /** ボーンマスクの範囲(上半身など)にベースを上書きするレイヤー */
        Upper,
        Max,
    };


    /**
     * @brief アニメーション制御クラス
     * @details k2EngineLow の Animation の代わりに ModelRender が持つ。
     *          クリップを親ローカルのTRSで扱うので、レイヤーのブレンドも親ローカル空間で行い、
     *          最後にモデル空間の行列へ変換して Skeleton に書き込む。
     *          Upper レイヤーは、ボーンマスクの範囲だけを Base に重ねる。マスク内のボーンは、マスク外の親(Base 側)の上にそのまま乗る。
     */
    class AnimationController
    {
    public:
        AnimationController();
        ~AnimationController() = default;


        /**
         * @brief 初期化
         * @details スケルトンのバインドポーズを保存してから、クリップを展開する。
         *          最初に Base レイヤーでクリップ 0 を再生する。
         * @note スケルトンの Update() や、このクラスの Update() より前に呼ぶこと(ボーンのローカル行列が上書きされる前の、バインドポーズが必要)。
         *       スケルトンとクリップは、このクラスより長く生きていること。
         * @param skeleton 結果を書き込むスケルトン。初期化済みであること
         * @param clips クリップの配列
         * @param clipNum クリップの数
         */
        void Init(Skeleton& skeleton, AnimationClip* clips, const int clipNum);


        /**
         * @brief 更新
         * @details 各レイヤーを進め、ブレンドし、スケルトンのボーン行列(モデル空間)を更新する。
         * @note スケルトンの Update() より前に呼ぶこと。
         * @param deltaTime 進める時間(秒)。再生速度をかけた後の値
         */
        void Update(const float deltaTime);


        /**
         * @brief クリップを再生する
         * @param layer 再生するレイヤー
         * @param clipIndex クリップの番号
         * @param fadeTime クロスフェードの時間(秒)。休止中の Upper では、レイヤーのフェードインの時間にもなる
         * @param isReplay 巻き戻して再生し直すか。その時はフェードしない
         */
        void Play(const EnAnimationLayer layer, const int clipIndex, const float fadeTime, const bool isReplay);


        /**
         * @brief レイヤーを止める
         * @details Base は止められないので何もしない。
         * @param layer 止めるレイヤー
         * @param fadeTime フェードアウトの時間(秒)
         */
        void Stop(const EnAnimationLayer layer, const float fadeTime);


        /**
         * @brief レイヤーでクリップが再生中か
         * @param layer 調べるレイヤー
         * @return 再生中なら true
         */
        bool IsPlaying(const EnAnimationLayer layer) const;


        /**
         * @brief レイヤーのボーンマスクを設定する
         * @details 指定したボーンとその子孫(サブツリー)の重みを 1、それ以外を 0 にする。
         *          初期値は全ボーン 1(レイヤーが全身を上書きする)。Base には設定できない。
         *          名前が重複するボーンは、最初に見つかったものを使う。
         *          keepRootOrientation が true なら、根のボーンの向きを、親(Base の歩きでねじれる骨盤)の向きに関わらず、このレイヤーのクリップのモデル空間の向きに保つ。
         *          上半身が、下半身の斜めのねじれにつられず、常に正面を向く。(animationpose::KeepModelOrientation)
         * @param layer 設定するレイヤー
         * @param rootBoneName マスクの根になるボーンの名前(例: 上半身なら背骨の最下段)
         * @param keepRootOrientation 根のボーンが、このレイヤーのクリップのモデル空間の向きを保つか(既定は false で、親ローカルの回転をそのまま重ねる)
         * @return 設定できたら true。ボーンが見つからない場合は false で、マスクは変わらない
         */
        bool SetLayerMask(const EnAnimationLayer layer, const wchar_t* rootBoneName, const bool keepRootOrientation = false);


        /**
         * @brief レイヤーが動いている間、Base のボーンの回転を、このレイヤーのクリップへ近づける
         * @details 例: 構え中、斜め歩きでねじれる骨盤を、構えたポーズの向きへ近づける。近づける割合は SetLayerFollowRatio()(初期値 0 で、何もしない)。
         *          レイヤーの出入りの重みを掛ける。ボーンの子のうち、レイヤーのマスクの外のもの(脚)は、モデル空間の向きを保つ。(animationpose::FollowBoneRotation)
         *          Base には設定できない。名前が重複するボーンは、最初に見つかったものを使う。
         * @param layer 設定するレイヤー
         * @param boneName 近づけるボーンの名前
         * @return 設定できたら true。ボーンが見つからない場合は false で、設定は変わらない
         */
        bool SetLayerFollowBone(const EnAnimationLayer layer, const wchar_t* boneName);


        /**
         * @brief SetLayerFollowBone() で決めたボーンを、レイヤーのクリップへ近づける割合を設定する
         * @param layer 設定するレイヤー
         * @param ratio 0〜1。0 なら何もしない。範囲外は丸められる
         */
        void SetLayerFollowRatio(const EnAnimationLayer layer, const float ratio);


        /**
         * @brief レイヤーのクリップの再生速度を設定する
         * @details クリップの時間とイベントの発火時刻にだけ掛ける。クロスフェードやレイヤーの出入りの長さは、実時間のまま。
         *          ModelRender の全体の再生速度は、Update() に渡す時間にすでに掛かっているので、その積になる。
         * @param layer 設定するレイヤー
         * @param speed 再生速度(1.0f=通常速度)
         */
        void SetLayerSpeed(const EnAnimationLayer layer, const float speed);


        /**
         * @brief アニメーションイベントのリスナーを登録する
         * @details 全レイヤーのイベントが、登録した全てのリスナーに通知される。
         * @param listener リスナー。引数はクリップ名とイベント名
         */
        void AddEventListener(const AnimationEventListener& listener);


        /**
         * @brief キーフレーム間の補間を設定する
         * @param isEnabled 補間するなら true(初期値)
         */
        void SetKeyframeInterpolation(const bool isEnabled);
        /**
         * @brief フェードのイージングを設定する
         * @param isEnabled smoothstep をかけるなら true(初期値)。false なら線形
         */
        void SetFadeEasing(const bool isEnabled);


    private:
        /** 結果を書き込むスケルトン */
        Skeleton* m_skeleton;
        /** クリップのサンプラー */
        std::vector<ClipSampler> m_samplers;
        /** バインドポーズ(親ローカル) */
        AnimationPose m_bindPose;
        /** ブレンド結果のポーズ */
        AnimationPose m_finalPose;
        /** ボーンごとの親のボーン番号 */
        std::vector<int> m_parentIndices;
        /** モデル空間の行列(作業用) */
        std::vector<Matrix> m_modelMatrices;
        /** マスクの根の向きを補正した、Upper のポーズ(作業用。毎フレーム使い回す) */
        AnimationPose m_adjustedUpperPose;
        /** レイヤー */
        std::array<AnimationLayer, static_cast<size_t>(EnAnimationLayer::Max)> m_layers;
        /** イベントのリスナー */
        std::vector<AnimationEventListener> m_listeners;
        /** ブレンドの設定 */
        AnimationBlendSettings m_settings;
        /** 初期化済みか */
        bool m_isInited;
    };
} // namespace nsBalloonEngine
