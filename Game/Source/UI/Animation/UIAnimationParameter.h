/**
 * @file UIAnimationParameter.h
 * @brief UIアニメーションの定義を、jsonから読み込んで保持する
 * @details jsonの"animations"に、アニメーションの定義を並べて書く。"key"は定義の名前で、Hash32した値でFind()する。
 *          jsonが更新されたら、定義を読み込み直す。(デバッグビルドのみ。HotReloadManagerを使っている。)
 *
 *          {
 *              "animations": [
 *                  { "key": "fadeIn", "valueType": "Vector4", "startValue": [255, 255, 255, 0], "endValue": [255, 255, 255, 255],
 *                    "duration": 0.5, "easing": "EaseOut", "loop": "Once" }
 *              ]
 *          }
 *
 *          "valueType"は、Float、Vector2、Vector3、Vector4。省略するとFloat。
 *          "startValue"と"endValue"は、Floatなら数値、Vector2〜4なら[x, y, ...]。Vector4は色として扱い、0〜255で書く。
 *          "duration"は秒(省略すると0.3)、"easing"はLinear / EaseIn / EaseOut / EaseInOut、"loop"はOnce / Loop / PingPong。
 *          "repeat"はLoop / PingPongの繰り返し回数(0で無限。省略すると0)、
 *          "endBehavior"はHold(終わった値で止まる) / Reset(始点に戻る)。省略するとHold。
 *
 *          NOTE: キーは、全てのjsonファイルで共通。同じキーが別のファイルにある場合は、後から読み込んだ方になる。
 */
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Source/Parameter/JsonView.h"
#include "Source/Util/Curve.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief UIアニメーションの定義
         * @details Float、Vector2、Vector3、Vector4の全ての値を持つ。valueTypeの型の値だけが、jsonから読み込まれる。
         */
        struct UIAnimationDef
        {
            /** 値の種類 */
            enum class ValueType : uint8_t
            {
                Float,
                Vector2,
                Vector3,
                Vector4
            };


            UIAnimationDef();


            /** 定義のキー(jsonの"key"をHash32した値) */
            uint32_t key;
            /** 値の種類 */
            ValueType valueType;

            /** 始める値と終わる値(Float) */
            float startFloat;
            float endFloat;
            /** 始める値と終わる値(Vector2) */
            Vector2 startV2;
            Vector2 endV2;
            /** 始める値と終わる値(Vector3) */
            Vector3 startV3;
            Vector3 endV3;
            /** 始める値と終わる値(Vector4。色) */
            Vector4 startV4;
            Vector4 endV4;

            /** 時間(秒) */
            float duration;
            /** イージングの種類 */
            util::EasingType easingType;
            /** ループの種類 */
            util::LoopMode loopMode;
            /** 繰り返す回数(Loop PingPong用)。util::InfiniteRepeatで無限 */
            uint16_t repeatCount;
            /** 終わった時の動作 */
            util::EndBehavior endBehavior;
        };




        /**
         * @brief UIアニメーションの定義を管理するクラス
         * @details Load()でjsonを読み込むと、定義がキーで引けるようになる。複数のjsonを読み込める。シングルトン。
         *          例: UIAnimationParameter::Get().Load("Assets/parameter/ui/Animation.json");
         *              const UIAnimationDef* def = UIAnimationParameter::Get().Find(Hash32("fadeIn"));
         *          NOTE: Find()で取り出したポインタは、次にjsonが読み込まれる(デバッグビルドでは保存される)まで使える。持ち続けないこと。
         */
        class UIAnimationParameter : public Noncopyable
        {
        private:
            struct FileEntry;

            /**
             * @brief 読み込んだ定義と、読み込んだファイル
             */
            struct Entry
            {
                /**
                 * @brief コンストラクタ
                 * @param def 定義
                 * @param fileIndex 読み込んだファイルの番号(m_filesの添え字)
                 */
                Entry(const UIAnimationDef& def, const size_t fileIndex)
                    : m_def(def)
                    , m_fileIndex(fileIndex)
                {
                }

                /** 定義 */
                UIAnimationDef m_def;
                /** 読み込んだファイルの番号 */
                size_t m_fileIndex;
            };


        private:
            UIAnimationParameter();
            ~UIAnimationParameter();


        public:
            /**
             * @brief jsonを読み込む
             * @details すぐに読み込む。すでに読み込んでいるjsonの場合は、何もしない。
             *          デバッグビルドでは、jsonが更新されたら読み込み直す。読み込めなかった場合は、直前の定義が残る。
             * @param path jsonファイルのパス
             * @return 読み込めたらtrue
             */
            bool Load(const std::string& path);

            /**
             * @brief 定義を取得
             * @param key キー(jsonの"key"をHash32した値)
             * @return 定義。無い場合はnullptr。
             */
            const UIAnimationDef* Find(const uint32_t key) const
            {
                const auto it = m_defs.find(key);
                return (it != m_defs.end()) ? &it->second.m_def : nullptr;
            }

            /**
             * @brief リビジョンを取得
             * @details 定義を読み込む度に増える。定義が変わったかどうかを調べるのに使う。
             * @return リビジョン
             */
            uint32_t GetRevision() const
            {
                return m_revision;
            }


        public:
            /**
             * @brief インスタンスを取得
             * @details 初回の呼び出し時に生成される。
             */
            static UIAnimationParameter& Get();


        private:
            /**
             * @brief jsonを読み込めた時の処理
             * @details このファイルから読み込んだ定義を入れ替える。"animations"が無い場合は、何も変えない。
             * @param fileIndex 読み込んだファイルの番号(m_filesの添え字)
             * @param root jsonのルート
             */
            void OnLoaded(const size_t fileIndex, const JsonView& root);


        private:
            /** 読み込んだファイル */
            std::vector<FileEntry> m_files;
            /** キーと定義の対応 */
            std::unordered_map<uint32_t, Entry> m_defs;
            /** リビジョン */
            uint32_t m_revision;
        };
    } // namespace ui
} // namespace app
