/**
 * @file ParamHolder.h
 * @brief パラメーターを保持して、IDで取り出せるようにするクラス
 * @details ParamList.cppに登録された全てのjsonファイルを、生成時にまとめて読み込んで保持する。シングルトン。
 *          jsonファイルの更新の監視と読み込み直しは、HotReloadManagerに任せる。(デバッグビルドでは、jsonを保存すると保持している値が書き換わる。)
 *          パラメーターは普通の構造体で、複数の場所から同じものを取り出せる。取り出した参照は、アプリが終わるまで使える。
 *          例: const MiniMapParameter& param = ParamHolder::Get().GetParameter<MiniMapParameter>(EnParamID::MiniMap);
 *          NOTE: ホットリロードで値が書き換わるので、値は毎回参照から読むこと。(必要なら、使うたびにコピーする。)
 *                配列のjsonをstd::vectorにしている場合も、要素へのポインタを次のフレームまで持たないこと。
 *          実行中の状態の確認は、ParamDebugUI(F3)で行える。
 */
#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include "Source/Parameter/HotReloadManager.h"
#include "Source/Parameter/JsonView.h"
#include "Source/Parameter/ParamList.h"


namespace app
{
    /**
     * @brief パラメーター1つ分の、デバッグ表示用の情報
     */
    struct ParamDebugInfo
    {
        /** 登録した型の名前(typeid(T).name()) */
        const char* m_typeName;
        /** jsonファイルのパス */
        const std::string* m_path;
        /** 1度でも読み込めたか */
        bool m_isLoaded;
        /** 最後の読み込みに失敗したか。(jsonが壊れている場合など。デバッグビルドのみ分かる。) */
        bool m_isLastLoadFailed;
        /** 読み込めた回数。(最初の読み込みも含む。) */
        uint32_t m_loadCount;
        /** 最後に読み込めてからの経過時間(秒)。1度も読み込めていない場合は-1。 */
        float m_secondsSinceLastLoad;
        /** 値の表示関数が登録されているか */
        bool m_hasDrawFunc;
    };


    /**
     * @brief パラメーターを保持するクラス
     */
    class ParamHolder : public Noncopyable
    {
    public:
        /**
         * @brief jsonからパラメーターの値を読み込む関数
         * @details paramは、デフォルト構築された状態で渡される。読み込めたら、保持している値と入れ替わる。
         */
        template <typename T>
        using LoadFunc = std::function<void(const JsonView& root, T& param)>;

        /**
         * @brief パラメーターの値をデバッグ表示する関数(省略可)
         * @details ParamDebugUIから呼ばれる。ImGuiの呼び出しは、BALLOON_IMGUI_ENABLEDで囲むこと。(Releaseにはない。)
         */
        template <typename T>
        using DrawFunc = std::function<void(const T& param)>;


    private:
        /**
         * @brief 登録されたパラメーター1つ分の基底クラス(型を消して保持するため)
         */
        class IEntry
        {
        public:
            virtual ~IEntry() = default;

            /** @brief 登録した型を取得 */
            virtual const std::type_info& GetType() const = 0;
            /** @brief パラメーターへのポインタを取得 */
            virtual const void* GetParameter() const = 0;
            /** @brief jsonを読み込めているかどうか */
            virtual bool IsLoaded() const = 0;
            /** @brief デバッグ表示用の情報を取得 */
            virtual ParamDebugInfo GetDebugInfo() const = 0;
            /** @brief 値の表示関数を呼ぶ。登録されていなければ何もしない。 */
            virtual void DrawValue() const = 0;
            /** @brief jsonを今すぐ読み込み直す */
            virtual bool Reload() = 0;
        };


        /**
         * @brief 登録されたパラメーター1つ分
         * @details パラメーターと、HotReloadManagerへの登録を持つ。破棄されると、登録を解除する。
         */
        template <typename T>
        class Entry : public IEntry
        {
        public:
            /**
             * @brief コンストラクタ
             * @details HotReloadManagerに登録して、すぐにjsonを読み込む。
             * @param path jsonファイルのパス
             * @param func jsonから値を読み込む関数
             * @param drawFunc 値の表示関数。空なら表示しない。
             */
            Entry(const std::string& path, const LoadFunc<T>& func, const DrawFunc<T>& drawFunc)
                : m_param()
                , m_isLoaded(false)
                , m_handle(INVALID_HOT_RELOAD_HANDLE)
                , m_path(path)
                , m_loadCount(0)
                , m_lastLoadTime()
                , m_drawFunc(drawFunc)
            {
                m_handle = HotReloadManager::Get().Register(path,
                    [this, func](const JsonView& root)
                    {
                        // 読み込めた時だけ値を入れ替える。(以前の値が残らないように、毎回デフォルト構築した状態から読み込む。)
                        T param{};
                        func(root, param);
                        m_param = std::move(param);
                        m_isLoaded = true;
                        ++m_loadCount;
                        m_lastLoadTime = std::chrono::steady_clock::now();
                    });
            }

            ~Entry() override
            {
                HotReloadManager::Get().Unregister(m_handle);
            }

            // NOTE: HotReloadManagerに登録した関数がthisを持つので、コピーもムーブもできない。
            Entry(const Entry&) = delete;
            Entry& operator=(const Entry&) = delete;

            const std::type_info& GetType() const override { return typeid(T); }
            const void* GetParameter() const override { return &m_param; }
            bool IsLoaded() const override { return m_isLoaded; }

            ParamDebugInfo GetDebugInfo() const override
            {
                ParamDebugInfo info = {};
                info.m_typeName = typeid(T).name();
                info.m_path = &m_path;
                info.m_isLoaded = m_isLoaded;
                info.m_isLastLoadFailed = HotReloadManager::Get().IsLastLoadFailed(m_handle);
                info.m_loadCount = m_loadCount;
                info.m_secondsSinceLastLoad = (m_loadCount == 0)
                    ? -1.0f
                    : std::chrono::duration<float>(std::chrono::steady_clock::now() - m_lastLoadTime).count();
                info.m_hasDrawFunc = static_cast<bool>(m_drawFunc);
                return info;
            }

            void DrawValue() const override
            {
                if (m_drawFunc)
                {
                    m_drawFunc(m_param);
                }
            }

            bool Reload() override
            {
                return HotReloadManager::Get().Reload(m_handle);
            }


        private:
            /** パラメーター */
            T m_param;
            /** jsonを読み込めているか */
            bool m_isLoaded;
            /** HotReloadManagerの登録ハンドル */
            HotReloadHandle m_handle;
            /** jsonファイルのパス */
            std::string m_path;
            /** 読み込めた回数 */
            uint32_t m_loadCount;
            /** 最後に読み込めた時刻 */
            std::chrono::steady_clock::time_point m_lastLoadTime;
            /** 値の表示関数 */
            DrawFunc<T> m_drawFunc;
        };


    private:
        /**
         * @brief コンストラクタ
         * @details RegisterAllParams()で、全てのパラメーターを登録して、jsonを読み込む。
         */
        ParamHolder();
        ~ParamHolder() = default;

        // 登録できるのは、一覧(ParamList.cpp)だけ
        friend void RegisterAllParams(ParamHolder& holder);


    public:
        /**
         * @brief パラメーターを取得する
         * @details jsonを読み込めていなくても、値(デフォルト構築された状態)を返す。読み込めているかは、IsLoaded()で調べる。
         *          登録されていないIDや、登録した型と違う型を指定した場合は、K2_ASSERTで止まる。
         *          止まらないビルドでは、デフォルト構築された値を返す。
         * @tparam T IDに登録した型
         * @param id パラメーターのID
         * @return パラメーターへの参照。アプリが終わるまで使える。
         */
        template <typename T>
        const T& GetParameter(EnParamID id) const
        {
            const IEntry* entry = FindEntry(id);
            if (entry == nullptr)
            {
                K2_ASSERT(false, "登録されていないIDが指定された。");
                return DefaultParameter<T>();
            }
            if (entry->GetType() != typeid(T))
            {
                K2_ASSERT(false, "IDに登録した型と違う型が指定された。");
                return DefaultParameter<T>();
            }
            return *static_cast<const T*>(entry->GetParameter());
        }


        /**
         * @brief jsonを読み込めているかどうか
         * @details 最初の読み込みに失敗した場合は、デバッグビルドで、jsonを直して保存すると読み込まれる。
         * @param id パラメーターのID
         * @return 1度でも読み込めていればtrue。登録されていないIDの場合はfalse。
         */
        bool IsLoaded(EnParamID id) const;


        /**
         * @brief デバッグ表示用の情報を取得する
         * @param id パラメーターのID
         * @param outInfo 情報の格納先
         * @return 取得できたらtrue。登録されていないIDの場合はfalse。(outInfoは変更しない。)
         */
        bool GetDebugInfo(EnParamID id, ParamDebugInfo& outInfo) const;


        /**
         * @brief パラメーターの値をデバッグ表示する
         * @details 登録された表示関数を呼ぶ。ImGuiのフレーム内で呼ぶこと。
         *          表示関数が登録されていない場合や、登録されていないIDの場合は、何もしない。
         * @param id パラメーターのID
         */
        void DrawValue(EnParamID id) const;


        /**
         * @brief jsonを今すぐ読み込み直す
         * @details 更新日時に関係なく読み込む。同じjsonファイルを使っている他のパラメーターも、一緒に更新される。
         *          デバッグ用。デバッグビルドでのみ動作する。リリースビルドでは何もしない。
         * @param id パラメーターのID
         * @return 読み込めたらtrue。登録されていないIDの場合や、リリースビルドではfalse。
         */
        bool Reload(EnParamID id);


    private:
        /**
         * @brief パラメーターを登録する
         * @details 登録するときに、すぐにjsonを読み込む。一覧(RegisterAllParams())からだけ呼ばれる。
         * @tparam T パラメーターの型
         * @param id パラメーターのID
         * @param path jsonファイルのパス
         * @param func jsonから値を読み込む関数
         * @param drawFunc 値の表示関数(省略可)。ParamDebugUIで、行を開くと呼ばれる。
         */
        template <typename T>
        void Register(EnParamID id, const std::string& path, const LoadFunc<T>& func, const DrawFunc<T>& drawFunc = nullptr)
        {
            const size_t index = static_cast<size_t>(id);
            if (index >= m_entries.size())
            {
                K2_ASSERT(false, "登録できないIDが指定された。");
                return;
            }
            K2_ASSERT(m_entries[index] == nullptr, "同じIDが二重に登録されている。");

            m_entries[index] = std::make_unique<Entry<T>>(path, func, drawFunc);
        }


        /**
         * @brief 登録されたパラメーターを探す
         * @return 見つかったパラメーター。登録されていない場合はnullptr。
         */
        const IEntry* FindEntry(EnParamID id) const;

        /** @brief 登録されたパラメーターを探す(const外し版) */
        IEntry* FindEntry(EnParamID id);


        /**
         * @brief デフォルト構築した値を取得する
         * @details 取得に失敗した場合に返す。
         */
        template <typename T>
        static const T& DefaultParameter()
        {
            static const T defaultParameter{};
            return defaultParameter;
        }


    private:
        /** 登録されたパラメーター。IDの値が、そのまま番号になる。 */
        std::array<std::unique_ptr<IEntry>, static_cast<size_t>(EnParamID::Max)> m_entries;


    public:
        /**
         * @brief インスタンスを生成する
         * @details 全てのパラメーターを読み込むので、ゲームの生成前に呼ぶこと。
         */
        static void CreateInstance();


        /**
         * @brief インスタンスを破棄する
         * @details 保持しているパラメーターを使うものがなくなってから呼ぶこと。
         */
        static void DestroyInstance();


        /**
         * @brief インスタンスを取得する
         * @return インスタンス。CreateInstance()を呼ぶ前に取得してはいけない。
         */
        static ParamHolder& Get();


    private:
        /** 唯一のインスタンス */
        static ParamHolder* m_instance;
    };
} // namespace app
