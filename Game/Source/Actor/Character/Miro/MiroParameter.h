/**
 * @file MiroParameter.h
 * @brief Miroのパラメーター
 * @details ParamHolderに EnParamID::Miro で登録する。jsonは Assets/parameter/actor/Miro.json。
 *          例: const MiroParameter& param = ParamHolder::Get().GetParameter<MiroParameter>(EnParamID::Miro);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief Miroのパラメーター
         * @note radius と height は Init() で一度だけ使うので、ホットリロードしても次に起動するまで反映されない。
         *       歩き・走り・構え中の速度と、構え中に体を回す角度と、上半身アニメーションのブレンド時間は、ホットリロードで変わる。
         *       maxHp はホットリロードで変わる。現在のHPは丸められないので、最大HPを下げると、現在のHPが最大HPを超えることがある。
         */
        struct MiroParameter
        {
            MiroParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 歩きの移動速度(入力の倒し具合が最大のときの速さ) */
            float walkSpeed;
            /** 走りの移動速度(入力の倒し具合が最大のときの速さ) */
            float runSpeed;
            /** 構え・リロード中の移動速度(入力の倒し具合が最大のときの速さ)。歩きより遅くする。下半身の歩きは、歩きとの速度の比だけ遅く再生される */
            float aimMoveSpeed;
            /** 構え中に、カメラの向きから画面の中央側(右)へ体を回す角度(度)。正で右。ハンドガンが見える角度に合わせる */
            float aimTurnDegrees;
            /** 構え・発砲・リロードの上半身アニメーションの、出入りと切り替えのブレンド時間(秒) */
            float upperBodyBlendTime;
            /** キャラクターコントローラーの半径 */
            float radius;
            /** キャラクターコントローラーの高さ */
            float height;
            /** 最大HP */
            float maxHp;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからMiroのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadMiroParameter(const JsonView& root, MiroParameter& param);

        /**
         * @brief Miroのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawMiroParameter(const MiroParameter& param);
    } // namespace actor
} // namespace app
