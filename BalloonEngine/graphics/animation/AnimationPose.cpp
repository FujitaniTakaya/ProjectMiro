/**
 * @file AnimationPose.cpp
 * @brief アニメーションのポーズのブレンド・変換の実装
 */
#include "BalloonEnginePreCompile.h"

#include "AnimationPose.h"

#include <algorithm>
#include <cmath>


namespace nsBalloonEngine
{
    namespace animationpose
    {
        namespace
        {
            /** これより小さい拡大の軸は、向きが決まらないものとして扱う */
            constexpr float MIN_SCALE = 1.0e-6f;
        } // namespace


        BoneTransform Decompose(const Matrix& matrix)
        {
            BoneTransform result;
            result.m_position = Vector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);

            // 行ベクトル規約なので、行 0〜2 が、拡大された X・Y・Z 軸になる
            float axis[3][3];
            float length[3];
            for (int i = 0; i < 3; ++i)
            {
                axis[i][0] = matrix.m[i][0];
                axis[i][1] = matrix.m[i][1];
                axis[i][2] = matrix.m[i][2];
                length[i] = std::sqrt(axis[i][0] * axis[i][0] + axis[i][1] * axis[i][1] + axis[i][2] * axis[i][2]);
            }

            // 3x3 の行列式が負なら、ミラーされている。x 軸の符号を拡大側へ移し、回転を正規の回転に保つ
            const float determinant =
                axis[0][0] * (axis[1][1] * axis[2][2] - axis[1][2] * axis[2][1]) - axis[0][1] * (axis[1][0] * axis[2][2] - axis[1][2] * axis[2][0]) + axis[0][2] * (axis[1][0] * axis[2][1] - axis[1][1] * axis[2][0]);
            const float mirror = (determinant < 0.0f) ? -1.0f : 1.0f;

            result.m_scale = Vector3(length[0] * mirror, length[1], length[2]);

            if (length[0] < MIN_SCALE || length[1] < MIN_SCALE || length[2] < MIN_SCALE)
            {
                // 軸が潰れていて回転が決まらない。0除算(NaN)を避けて、回転は単位のままにする
                return result;
            }

            Matrix rotation = Matrix::Identity;
            for (int i = 0; i < 3; ++i)
            {
                const float sign = (i == 0) ? mirror : 1.0f;
                for (int j = 0; j < 3; ++j)
                {
                    rotation.m[i][j] = axis[i][j] / length[i] * sign;
                }
            }
            result.m_rotation.SetRotation(rotation);
            return result;
        }


        Matrix Compose(const BoneTransform& transform)
        {
            Matrix scaleMatrix;
            scaleMatrix.MakeScaling(transform.m_scale);
            Matrix rotationMatrix;
            rotationMatrix.MakeRotationFromQuaternion(transform.m_rotation);
            Matrix translationMatrix;
            translationMatrix.MakeTranslation(transform.m_position);

            Matrix result = scaleMatrix * rotationMatrix;
            result = result * translationMatrix;
            return result;
        }


        float SmoothStep(const float t)
        {
            return t * t * (3.0f - 2.0f * t);
        }


        BoneTransform Blend(const BoneTransform& a, const BoneTransform& b, const float t)
        {
            if (t <= 0.0f)
            {
                return a;
            }
            if (t >= 1.0f)
            {
                return b;
            }

            BoneTransform result;
            result.m_position.Lerp(t, a.m_position, b.m_position);
            result.m_scale.Lerp(t, a.m_scale, b.m_scale);

            // XMQuaternionSlerp は、q と -q が同じ回転であることを見て、最短経路で補間する(ハーネスで確認済み)
            result.m_rotation.Slerp(t, a.m_rotation, b.m_rotation);
            return result;
        }


        void BlendPose(AnimationPose& result, const AnimationPose& a, const AnimationPose& b, const float t)
        {
            const size_t boneCount = (std::min)(a.size(), b.size());
            if (result.size() != boneCount)
            {
                result.resize(boneCount);
            }
            for (size_t i = 0; i < boneCount; ++i)
            {
                // a・b と result が同じ配列でも、1ボーンずつ計算して書くので問題ない
                result[i] = Blend(a[i], b[i], t);
            }
        }


        void OverlayPose(AnimationPose& base, const AnimationPose& overlay, const std::vector<float>& weights, const float layerWeight)
        {
            const size_t boneCount = (std::min)((std::min)(base.size(), overlay.size()), weights.size());
            for (size_t i = 0; i < boneCount; ++i)
            {
                const float weight = weights[i] * layerWeight;
                if (weight <= 0.0f)
                {
                    continue;
                }
                base[i] = Blend(base[i], overlay[i], (std::min)(weight, 1.0f));
            }
        }


        void ToModelSpace(const AnimationPose& pose, const std::vector<int>& parentIndices, std::vector<Matrix>& modelMatrices)
        {
            const size_t boneCount = pose.size();
            modelMatrices.resize(boneCount);
            for (size_t i = 0; i < boneCount; ++i)
            {
                const Matrix localMatrix = Compose(pose[i]);
                const int parent = parentIndices[i];
                // 親が先に並んでいる前提(parent < i)なので、親のモデル行列は計算済み
                modelMatrices[i] = (parent < 0) ? localMatrix : (localMatrix * modelMatrices[parent]);
            }
        }
    } // namespace animationpose
} // namespace nsBalloonEngine
