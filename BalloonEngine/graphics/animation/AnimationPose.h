/**
 * @file AnimationPose.h
 * @brief アニメーションのポーズ(ボーンごとの親ローカルTRS)と、そのブレンド・変換の宣言
 */
#pragma once
#include <vector>


namespace nsBalloonEngine
{
    /**
     * @brief 1本のボーンの親ローカルのトランスフォーム
     * @details 行列はスケール→回転→平行移動の順(行ベクトル規約)で組み立てる。
     */
    struct BoneTransform
    {
        /** 親ローカルの座標 */
        Vector3 m_position;
        /** 親ローカルの回転 */
        Quaternion m_rotation;
        /** 親ローカルの拡大。ミラーしたボーンは x が負になる */
        Vector3 m_scale;


        BoneTransform()
            : m_position(Vector3::Zero)
            , m_rotation(Quaternion::Identity)
            , m_scale(Vector3::One)
        {}
    };


    /** 全ボーンの親ローカルトランスフォーム。添字はスケルトンのボーン番号 */
    using AnimationPose = std::vector<BoneTransform>;


    /**
     * @brief アニメーションブレンドの設定
     */
    struct AnimationBlendSettings
    {
        /** キーフレーム間を補間するか(false なら k2EngineLow の Animation と同じコマ送り) */
        bool m_interpolateKeyframes;
        /** クロスフェードとレイヤーの出入りに、smoothstep のイージングをかけるか(false なら線形) */
        bool m_easeFade;


        AnimationBlendSettings()
            : m_interpolateKeyframes(true)
            , m_easeFade(true)
        {}
    };


    /**
     * @brief ポーズの変換・ブレンドの関数群
     * @details エンジンの型(スケルトン・クリップ)に依存しない数学だけを置く。
     */
    namespace animationpose
    {
        /**
         * @brief 親ローカル行列を、位置・回転・拡大に分解する
         * @details 行列式が負(ミラー)の場合は、拡大の x を負にして回転を正規の回転に保つ。
         *          拡大が0に近い軸があると回転が決まらないので、その時の回転は単位回転にする。
         * @param matrix 親ローカル行列(行ベクトル規約)
         * @return 分解したトランスフォーム
         */
        BoneTransform Decompose(const Matrix& matrix);


        /**
         * @brief 位置・回転・拡大から、親ローカル行列を作る
         * @param transform トランスフォーム
         * @return 親ローカル行列(スケール→回転→平行移動)
         */
        Matrix Compose(const BoneTransform& transform);


        /**
         * @brief smoothstep
         * @param t 0〜1
         * @return イージングをかけた値(0〜1)
         */
        float SmoothStep(const float t);


        /**
         * @brief 2つのトランスフォームをブレンドする
         * @details 位置・拡大は線形補間、回転は球面線形補間(q と -q は同じ回転として、最短経路で補間する)。t が 0 以下なら a、1 以上なら b をそのまま返す。
         * @param a t=0 側
         * @param b t=1 側
         * @param t ブレンド率(0〜1)
         * @return ブレンド結果
         */
        BoneTransform Blend(const BoneTransform& a, const BoneTransform& b, const float t);


        /**
         * @brief 2つのポーズを、全ボーン同じ率でブレンドする
         * @param[out] result 結果。a や b と同じオブジェクトでもよい
         * @param a t=0 側のポーズ
         * @param b t=1 側のポーズ
         * @param t ブレンド率(0〜1)
         */
        void BlendPose(AnimationPose& result, const AnimationPose& a, const AnimationPose& b, const float t);


        /**
         * @brief ポーズに、別のポーズをボーンごとの重みで重ねる
         * @details ボーン i の重みは weights[i] * layerWeight。0以下のボーンは変更しない。
         * @param[in,out] base 重ねられる側(土台)のポーズ
         * @param overlay 重ねるポーズ
         * @param weights ボーンごとの重み(0〜1)。ボーン数と同じ長さ
         * @param layerWeight レイヤー全体の重み(0〜1)
         */
        void OverlayPose(AnimationPose& base, const AnimationPose& overlay, const std::vector<float>& weights, const float layerWeight);


        /**
         * @brief 親ローカルのポーズを、モデル空間(ルートボーン空間)の行列に変換する
         * @note 親のボーン番号は、子より小さいこと(親が先に並んでいること)。
         * @param pose 親ローカルのポーズ
         * @param parentIndices ボーンごとの親のボーン番号。ルートは -1
         * @param[out] modelMatrices モデル空間の行列。ボーン数に resize される
         */
        void ToModelSpace(const AnimationPose& pose, const std::vector<int>& parentIndices, std::vector<Matrix>& modelMatrices);
    } // namespace animationpose
} // namespace nsBalloonEngine
