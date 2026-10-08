/**
 * @file UIAnimationParameter.cpp
 * @brief UIアニメーションの定義を、jsonから読み込んで保持する
 */
#include "stdafx.h"

#include "UIAnimationParameter.h"

#include <algorithm>

#include "Source/Parameter/HotReloadManager.h"
#include "Source/Parameter/ParamLoader.h"
#include "Source/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** 時間を省略した時の値(秒) */
            constexpr float DEFAULT_DURATION = 0.3f;
            /** 繰り返す回数の上限。uint16_tに収まる値 */
            constexpr int MAX_REPEAT_COUNT = 65535;


            /** 文字列から、値の種類を取得する。知らない文字列はFloat */
            UIAnimationDef::ValueType ToValueType(const std::string& name)
            {
                if (name == "Vector2")
                {
                    return UIAnimationDef::ValueType::Vector2;
                }
                if (name == "Vector3")
                {
                    return UIAnimationDef::ValueType::Vector3;
                }
                if (name == "Vector4")
                {
                    return UIAnimationDef::ValueType::Vector4;
                }
                if (!name.empty() && name != "Float" && name != "float")
                {
                    K2_LOG("UIアニメーションの\"valueType\"が未対応です。Floatとして扱います。valueType=%s\n", name.c_str());
                }
                return UIAnimationDef::ValueType::Float;
            }


            /**
             * @brief jsonから、定義1つ分を読み込む
             * @details NOTE: 省略された時の値は、Vector3::Zeroなどのstatic変数ではなく、リテラルで渡している。
             *                (他のファイルのstatic変数の初期化中に呼ばれると、ゼロのままになるため。)
             * @param item 定義1つ分のjson
             * @param def 読み込み先。keyは呼び出し側で設定済みであること。
             */
            void ReadDefinition(const JsonView& item, UIAnimationDef& def)
            {
                def.valueType = ToValueType(ParamLoader::ToString(item, "valueType"));
                switch (def.valueType)
                {
                case UIAnimationDef::ValueType::Vector2:
                    def.startV2 = ParamLoader::ToVector2(item, "startValue", Vector2(0.0f, 0.0f));
                    def.endV2 = ParamLoader::ToVector2(item, "endValue", Vector2(0.0f, 0.0f));
                    break;
                case UIAnimationDef::ValueType::Vector3:
                    def.startV3 = ParamLoader::ToVector3(item, "startValue", Vector3(0.0f, 0.0f, 0.0f));
                    def.endV3 = ParamLoader::ToVector3(item, "endValue", Vector3(0.0f, 0.0f, 0.0f));
                    break;
                case UIAnimationDef::ValueType::Vector4:
                    // 色として扱う。0〜255を0〜1にする。
                    def.startV4 = ParamLoader::ToVector4(item, "startValue", true, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
                    def.endV4 = ParamLoader::ToVector4(item, "endValue", true, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
                    break;
                case UIAnimationDef::ValueType::Float:
                default:
                    def.startFloat = ParamLoader::ToFloat(item, "startValue", 0.0f);
                    def.endFloat = ParamLoader::ToFloat(item, "endValue", 0.0f);
                    break;
                }

                def.duration = ParamLoader::ToFloat(item, "duration", DEFAULT_DURATION);
                def.easingType = util::ToEasingType(ParamLoader::ToString(item, "easing"));
                def.loopMode = util::ToLoopMode(ParamLoader::ToString(item, "loop"));

                const int repeat = ParamLoader::ToInt(item, "repeat", 0);
                def.repeatCount = static_cast<uint16_t>(std::clamp(repeat, 0, MAX_REPEAT_COUNT));

                def.endBehavior = (ParamLoader::ToString(item, "endBehavior") == "Reset")
                                      ? util::EndBehavior::Reset
                                      : util::EndBehavior::Hold;
            }
        } // namespace




        //=======================================================================
        // UIAnimationDef
        //=======================================================================
        // NOTE: 色などの初期値は、Vector4::Whiteなどのstatic変数ではなく、リテラルで初期化している。
        UIAnimationDef::UIAnimationDef()
            : key(0)
            , valueType(ValueType::Float)
            , startFloat(0.0f)
            , endFloat(0.0f)
            , startV2(0.0f, 0.0f)
            , endV2(0.0f, 0.0f)
            , startV3(0.0f, 0.0f, 0.0f)
            , endV3(0.0f, 0.0f, 0.0f)
            , startV4(1.0f, 1.0f, 1.0f, 1.0f)
            , endV4(1.0f, 1.0f, 1.0f, 1.0f)
            , duration(DEFAULT_DURATION)
            , easingType(util::EasingType::Linear)
            , loopMode(util::LoopMode::Once)
            , repeatCount(util::InfiniteRepeat)
            , endBehavior(util::EndBehavior::Hold)
        {}




        //=======================================================================
        // UIAnimationParameter
        //=======================================================================
        /**
         * @brief 読み込んだファイルの情報
         */
        struct UIAnimationParameter::FileEntry
        {
            /**
             * @brief コンストラクタ
             * @param path jsonファイルのパス
             */
            explicit FileEntry(const std::string& path)
                : m_path(path)
                , m_handle(INVALID_HOT_RELOAD_HANDLE)
                , m_isLoaded(false)
                , m_keys()
            {
            }

            /** jsonファイルのパス */
            std::string m_path;
            /** HotReloadManagerに登録したハンドル */
            HotReloadHandle m_handle;
            /** 1度でも読み込めたか */
            bool m_isLoaded;
            /** このファイルから読み込んだ定義のキー */
            std::vector<uint32_t> m_keys;
        };


        UIAnimationParameter::UIAnimationParameter()
            : m_files()
            , m_defs()
            , m_revision(0)
        {}


        // NOTE: HotReloadManagerも関数内のstatic変数なので、ここでUnregister()は呼ばない。
        //       登録した関数はこのクラスのポインタを持つが、このクラスはアプリが終わるまで破棄されない。
        UIAnimationParameter::~UIAnimationParameter()
        {}


        UIAnimationParameter& UIAnimationParameter::Get()
        {
            static UIAnimationParameter instance;
            return instance;
        }


        bool UIAnimationParameter::Load(const std::string& path)
        {
            // NOTE: 表記の違い(「a/b.json」と「a\b.json」、大文字小文字)があっても、同じファイルとして扱う。
            //       HotReloadManagerが同じファイルを1つにまとめる判定と、同じにしておく。
            const std::string key = HotReloadManager::NormalizePath(path);
            for (const FileEntry& file : m_files)
            {
                if (HotReloadManager::NormalizePath(file.m_path) == key)
                {
                    return file.m_isLoaded;
                }
            }

            // NOTE: Register()は、登録した時に1回、すぐにOnLoaded()を呼ぶ。先にm_filesに入れておくこと。
            const size_t fileIndex = m_files.size();
            m_files.emplace_back(path);
            const HotReloadHandle handle = HotReloadManager::Get().Register(
                path,
                [this, fileIndex](const JsonView& root) { OnLoaded(fileIndex, root); }
            );
            m_files[fileIndex].m_handle = handle;
            return m_files[fileIndex].m_isLoaded;
        }


        void UIAnimationParameter::OnLoaded(const size_t fileIndex, const JsonView& root)
        {
            if (!root.Contains("animations"))
            {
                K2_LOG("UIアニメーションのjsonに\"animations\"がありません。path=%s\n", m_files[fileIndex].m_path.c_str());
                return;
            }

            // 先に全て読み込んでから入れ替える。
            const JsonView animations = root.Get("animations");
            std::vector<UIAnimationDef> parsed;
            parsed.reserve(animations.Size());
            for (size_t i = 0; i < animations.Size(); ++i)
            {
                const JsonView item = animations[i];
                const std::string name = ParamLoader::ToString(item, "key");
                if (name.empty())
                {
                    K2_LOG("UIアニメーションの定義に\"key\"がありません。path=%s\n", m_files[fileIndex].m_path.c_str());
                    continue;
                }

                UIAnimationDef def;
                def.key = Hash32(name.c_str());
                ReadDefinition(item, def);
                parsed.push_back(def);
            }

            FileEntry& file = m_files[fileIndex];

            // このファイルから以前読み込んだ定義を外す。(jsonから消されたキーが、残らないようにするため。)
            // 別のファイルが同じキーで上書きしている場合は、そちらを残す。
            for (const uint32_t key : file.m_keys)
            {
                const auto it = m_defs.find(key);
                if (it != m_defs.end() && it->second.m_fileIndex == fileIndex)
                {
                    m_defs.erase(it);
                }
            }
            file.m_keys.clear();

            for (const UIAnimationDef& def : parsed)
            {
                const auto it = m_defs.find(def.key);
                if (it != m_defs.end() && it->second.m_fileIndex != fileIndex)
                {
                    K2_LOG("UIアニメーションのキーが、別のファイルと重複しています。後から読み込んだ方になります。key=%u path=%s\n", def.key, file.m_path.c_str());
                }
                m_defs.insert_or_assign(def.key, Entry(def, fileIndex));
                file.m_keys.push_back(def.key);
            }

            file.m_isLoaded = true;
            ++m_revision;
        }
    } // namespace ui
} // namespace app
