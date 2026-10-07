/**
 * @file HotReloadManager.h
 * @brief jsonファイルのホットリロードをまとめて管理する
 */
#pragma once
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "Source/Parameter/JsonView.h"


namespace app
{
    /** ホットリロードの登録ハンドル */
    using HotReloadHandle = uint32_t;
    /** ハンドル無効値 */
    static constexpr HotReloadHandle INVALID_HOT_RELOAD_HANDLE = 0xffffffff;


    /**
     * @brief jsonを読み込んだ時に呼ばれる関数
     * @details 引数のJsonViewは、この関数の中でだけ使える。値は関数の中でコピーして保持すること。
     */
    using HotReloadCallback = std::function<void(const JsonView& root)>;


    /**
     * @brief jsonファイルのホットリロードを管理するクラス
     * @details ファイルのパスと「読み込めた時に呼ぶ関数」を登録しておくと、
     *          Update()を呼ぶだけで、登録した全てのファイルの更新を調べ、更新されていれば読み込み直して関数を呼ぶ。
     *          ファイルの読み込みはJsonLoaderで行う。
     *          同じファイルに複数の関数を登録した場合も、更新の確認も読み込み直しもファイルごとに1回だけで、
     *          同じJsonViewを登録された全ての関数に渡す。
     *          例: m_handle = HotReloadManager::Get().Register("Assets/Param/Enemy.json",
     *                  [this](const JsonView& root) { LoadEnemy(root); });
     *              HotReloadManager::Get().Unregister(m_handle);
     *          NOTE: ファイルの監視はデバッグビルドでのみ行う。リリースビルドでは、Register()での最初の読み込みだけ行う。
     *          NOTE: 同じファイルかどうかは、パスの表記(「a/b.json」と「a\b.json」など)の違いは無視して判断する。
     *                大文字小文字の違いは、別のファイルとして扱う。
     *          NOTE: 関数内のstatic変数で保持するので、他のstatic変数のデストラクタからUnregister()を呼ばないこと。
     */
    class HotReloadManager : public Noncopyable
    {
    private:
        /**
         * @brief ファイルを読み込んだ時に呼ぶ関数の登録
         */
        struct Listener
        {
            /**
             * @brief コンストラクタ
             * @param handle 登録ハンドル
             * @param callback 読み込めた時に呼ぶ関数
             */
            Listener(HotReloadHandle handle, const HotReloadCallback& callback)
                : m_handle(handle)
                , m_callback(callback)
            {
            }

            /** 登録ハンドル */
            HotReloadHandle m_handle;
            /** 読み込めた時に呼ぶ関数 */
            HotReloadCallback m_callback;
        };


        /**
         * @brief 登録されたファイルの情報
         */
        struct FileEntry
        {
            /**
             * @brief コンストラクタ
             * @param path jsonファイルのパス
             */
            explicit FileEntry(const std::string& path)
                : m_path(path)
                , m_listeners()
#ifdef _DEBUG
                , m_lastWriteTime((std::filesystem::file_time_type::min)())
                , m_isLastLoadFailed(false)
#endif // _DEBUG
            {
            }

            /** jsonファイルのパス */
            std::string m_path;
            /** このファイルを読み込んだ時に呼ぶ関数 */
            std::vector<Listener> m_listeners;
#ifdef _DEBUG
            /** 最後に読み込めた時の、ファイルの更新日時。まだ読み込めていない場合は最小値。 */
            std::filesystem::file_time_type m_lastWriteTime;
            /** 最後の読み込みに失敗したかどうか */
            bool m_isLastLoadFailed;
#endif // _DEBUG
        };


    private:
        HotReloadManager();
        ~HotReloadManager() = default;


    public:
        /**
         * @brief 更新処理
         * @details 登録されたファイルが更新されていれば読み込み直し、登録された関数を呼ぶ。
         *          更新を調べるのは、SetCheckInterval()で設定した間隔ごと。
         *          ファイルの読み込みに失敗した場合は関数を呼ばず、次に調べる時にもう一度読み込みを試す。
         *          関数の中から、Register()やUnregister()を呼んでもよい。
         *          デバッグビルドでのみ動作する。リリースビルドでは何もしない。
         *          毎フレーム呼ぶこと。(Application::PreUpdate()から呼ぶ。)
         */
        void Update();


        /**
         * @brief ファイルを登録する
         * @details すぐにファイルを読み込み、読み込めたら関数を1回呼ぶ。(リリースビルドでも呼ぶ。)
         *          この最初の読み込みは、登録ごとに行う。(同じファイルを複数登録しても共有しない。)
         *          デバッグビルドでは、読み込めなかった場合も登録は残り、読み込めるようになった時に関数を呼ぶ。
         * @param path jsonファイルのパス
         * @param callback 読み込めた時に呼ぶ関数
         * @return ハンドル。関数が空の場合はINVALID_HOT_RELOAD_HANDLE。
         */
        HotReloadHandle Register(const std::string& path, HotReloadCallback callback);


        /**
         * @brief ファイルの登録を解除する
         * @details 関数の持ち主が破棄される前に、必ず呼ぶこと。
         * @param handle Register()で受け取ったハンドル。登録されていない場合は何もしない。
         */
        void Unregister(HotReloadHandle handle);


        /**
         * @brief ファイルを、今すぐ読み込み直す
         * @details ファイルの更新日時に関係なく読み込み、読み込めたら、そのファイルに登録された全ての関数を呼ぶ。
         *          読み込めなかった場合は、関数を呼ばない。(Update()で更新を見つけた時と同じ。)
         *          デバッグ用。デバッグビルドでのみ動作する。リリースビルドでは何もしない。
         * @param handle Register()で受け取ったハンドル
         * @return 読み込めたらtrue。登録されていない場合や、リリースビルドではfalse。
         */
        bool Reload(HotReloadHandle handle);


        /**
         * @brief ファイルの最後の読み込みに失敗したかどうか
         * @details 保存の途中や、jsonの書き間違いで読み込めなかった時にtrueになり、次に読み込めるとfalseに戻る。
         *          Register()での最初の読み込みも含む。デバッグ用。
         * @param handle Register()で受け取ったハンドル
         * @return 失敗していたらtrue。登録されていない場合や、リリースビルド(監視しないので分からない)ではfalse。
         */
        bool IsLastLoadFailed(HotReloadHandle handle) const;


        /**
         * @brief 更新を調べる間隔を設定する
         * @param seconds 間隔(秒)。初期値は0.5秒。
         */
        void SetCheckInterval(float seconds);


    public:
        /**
         * @brief インスタンスを取得
         * @details 初回の呼び出し時に生成される。
         */
        static HotReloadManager& Get();


    private:
        /**
         * @brief 1つのファイルを読み込み直して、登録された関数を呼ぶ
         * @details ファイルの読み込みは、登録された関数の数にかかわらず1回だけ。
         *          読み込めなかった場合は、更新日時を記録しない。(次に調べる時にも、もう一度試す。)
         *          デバッグビルドでのみ動作する。リリースビルドでは何もしない。
         * @param key m_filesのキー
         * @return 読み込めて、関数を呼んだらtrue
         */
        bool ReloadFile(const std::string& key);


    private:
        /** パスと登録されたファイルの対応(デバッグビルドのみ)。パスは、表記の違いを無くした形をキーにする。 */
        std::map<std::string, FileEntry> m_files;
        /** ハンドルと、登録されたファイルのキーの対応(デバッグビルドのみ) */
        std::map<HotReloadHandle, std::string> m_handleToKey;
        /** 次に発行するハンドル */
        HotReloadHandle m_nextHandle;
        /** 更新を調べる間隔(秒) */
        float m_checkInterval;
#ifdef _DEBUG
        /** 最後に更新を調べた時刻 */
        std::chrono::steady_clock::time_point m_lastCheckTime;
#endif // _DEBUG
    };
} // namespace app
