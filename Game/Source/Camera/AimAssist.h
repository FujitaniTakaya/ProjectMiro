/**
 * @file AimAssist.h
 * @brief 構え中の狙いやすさを助ける判定(エイムアシスト)
 * @details カメラの視線の近くにいるゾンビがあると、右スティックの回転を遅くしたり、視線を狙う点へ寄せたりするための数値を返す。
 *          PlayerCamera が、構えている間だけ使う。
 */
#pragma once
#include <vector>


namespace app
{
    namespace camera
    {
        /** 前方宣言 */
        struct AimAssistParameter;


        /**
         * @brief AimAssist::Evaluate() の結果
         * @details 対象がいない時は、hasTarget=false、weight=0、slowScale=1、誤差0。
         */
        struct AimAssistResult
        {
            AimAssistResult();


            /** アシストの対象がいるか */
            bool hasTarget;
            /** 視線が狙う点にどれだけ近いか(0=範囲の縁、1=ど真ん中) */
            float weight;
            /** 回転速度の倍率(1=変えない)。weight が大きいほど小さい */
            float slowScale;
            /** 狙う点が、視線より右にある角度(度)。左なら負 */
            float yawErrorDeg;
            /** 狙う点が、視線より上にある角度(度)。下なら負 */
            float pitchErrorDeg;
        };


        /**
         * @brief 構え中の狙いやすさを助ける判定
         * @details カメラの視線から一番近い「狙う点」を1つ選び、近さ(weight)に応じた減速の倍率と、点までの角度の誤差を返す。
         *          状態を持たない。アクターもエンジンも知らず、座標だけを受け取る。
         *          調整値は AimAssistParameter(ParamHolder が持つもの。ホットリロードで値が変わる)。
         *          例: AimAssist assist;
         *              const AimAssistResult result = assist.Evaluate(cameraPosition, cameraForward, targets);
         */
        class AimAssist : public Noncopyable
        {
        public:
            AimAssist();
            ~AimAssist();


            /**
             * @brief 視線に一番近い狙う点を選んで、結果を返す
             * @details 次の点が対象。
             *          - カメラの前にある(視線方向の奥行きが正)
             *          - カメラから maxDistance 以内
             *          - 視線に垂直な距離(視線から点までの距離)が assistRadius より小さい
             *          対象の中で、視線に垂直な距離が最も小さい点を選ぶ。weight は、その距離から滑らかに決める。(ど真ん中で1、assistRadius で0)
             *          パラメーターは毎回読むので、jsonのホットリロードがすぐ反映される。
             * @note カメラがほぼ真上・真下を向いている時は、左右の向きが決まらないので、対象なしにする。
             * @param cameraPosition カメラの位置
             * @param cameraForward カメラの前方向(大きさは問わない)
             * @param targets 狙う点(ゾンビの胸の座標)
             * @return 結果。対象が無ければ hasTarget=false、slowScale=1
             */
            AimAssistResult Evaluate(const Vector3& cameraPosition, const Vector3& cameraForward, const std::vector<Vector3>& targets) const;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const AimAssistParameter& m_parameter;
        };
    } // namespace camera
} // namespace app
