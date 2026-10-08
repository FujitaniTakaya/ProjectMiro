/**
 * @file HotReloadManager.cpp
 * @brief jsonファイルのホットリロードをまとめて管理する
 */
#include "stdafx.h"

#include "HotReloadManager.h"

#include <algorithm>
#include <system_error>

#include "JsonLoader.h"


namespace app
{
    namespace
    {
        /** 更新を調べる間隔の初期値(秒) */
        constexpr float DEFAULT_CHECK_INTERVAL = 0.5f;
    } // namespace


    HotReloadManager::HotReloadManager()
        : m_files()
        , m_handleToKey()
        , m_nextHandle(0)
        , m_checkInterval(DEFAULT_CHECK_INTERVAL)
#ifdef K2_DEBUG
        , m_lastCheckTime(std::chrono::steady_clock::now())
#endif // K2_DEBUG
    {
    }


    void HotReloadManager::Update()
    {
#ifdef K2_DEBUG
        // 調べる間隔が経っていなければ、何もしない
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = now - m_lastCheckTime;
        if (elapsed.count() < m_checkInterval)
        {
            return;
        }
        m_lastCheckTime = now;

        // 更新されたファイルを集める。ファイルごとに1回だけ更新日時を調べる。
        // NOTE: 関数の中でRegister()やUnregister()が呼ばれてもいいように、
        //       m_filesを回しながらではなく、集めてから読み込む。
        std::vector<std::string> updatedKeys;
        for (const auto& [key, file] : m_files)
        {
            std::error_code ec;
            const auto writeTime = std::filesystem::last_write_time(file.m_path, ec);
            // ファイルがない場合は、更新されていないものとして扱う
            if (ec)
            {
                continue;
            }

            // NOTE: 古い日時のファイルに差し替えられた場合も読み込み直すため、「新しい」ではなく「違う」で調べる。
            if (writeTime != file.m_lastWriteTime)
            {
                updatedKeys.push_back(key);
            }
        }

        for (const std::string& key : updatedKeys)
        {
            // NOTE: 他のファイルの関数でUnregister()されて、なくなっていた場合は、ReloadFile()の中で飛ばされる。
            ReloadFile(key);
        }
#endif // K2_DEBUG
    }


    bool HotReloadManager::ReloadFile(const std::string& key)
    {
#ifdef K2_DEBUG
        const auto it = m_files.find(key);
        if (it == m_files.end())
        {
            return false;
        }

        // 更新日時は、読み込む前に取得する。
        // (読み込み中に保存されても、次に調べる時に更新を見つけられる。)
        std::error_code ec;
        const auto writeTime = std::filesystem::last_write_time(it->second.m_path, ec);

        // 保存の途中などで読み込めない場合は、更新日時を記録しない。次に調べる時にもう一度試す。
        // NOTE: ファイルの読み込みは、登録された関数の数にかかわらず1回だけ。
        JsonLoader loader;
        if (ec || !loader.Load(it->second.m_path))
        {
            it->second.m_isLastLoadFailed = true;
            return false;
        }

        it->second.m_lastWriteTime = writeTime;
        it->second.m_isLastLoadFailed = false;

        // NOTE: 関数の中でUnregister()されても、実行中の関数が破棄されないように、コピーした方から呼ぶ。
        //       コピーした後はitを使わない。
        const std::vector<Listener> listeners = it->second.m_listeners;
        const JsonView root = loader.GetRoot();
        for (const Listener& listener : listeners)
        {
            // 先に呼んだ関数でUnregister()された場合は、飛ばす
            if (m_handleToKey.find(listener.m_handle) == m_handleToKey.end())
            {
                continue;
            }

            listener.m_callback(root);
        }
        return true;
#else
        (void)key;
        return false;
#endif // K2_DEBUG
    }


    HotReloadHandle HotReloadManager::Register(const std::string& path, HotReloadCallback callback)
    {
        if (!callback)
        {
            return INVALID_HOT_RELOAD_HANDLE;
        }

        // ハンドルが最大数になったら使えない
        // NOTE: そんなに登録するはずがない
        if (m_nextHandle == INVALID_HOT_RELOAD_HANDLE)
        {
            K2_ASSERT(false, "ホットリロードの登録が多いです。\n");
            return INVALID_HOT_RELOAD_HANDLE;
        }

        const HotReloadHandle handle = m_nextHandle++;

#ifdef K2_DEBUG
        // 更新日時は、読み込む前に取得する
        std::error_code ec;
        const auto writeTime = std::filesystem::last_write_time(path, ec);
#endif // K2_DEBUG

        JsonLoader loader;
        const bool isLoaded = loader.Load(path);
        if (isLoaded)
        {
            callback(loader.GetRoot());
        }

#ifdef K2_DEBUG
        // NOTE: 関数の中でRegister()されることもあるので、関数を呼んだ後で登録する。
        const std::string key = NormalizePath(path);
        auto it = m_files.find(key);
        if (it == m_files.end())
        {
            it = m_files.emplace(key, FileEntry(path)).first;

            // 新しく登録したファイルだけ、読み込めた時の更新日時を記録する。
            // NOTE: 既に登録されているファイルの更新日時は変えない。
            //       まだ読み込めていない関数が、次の更新で読み込み直される機会をなくさないため。
            if (isLoaded && !ec)
            {
                it->second.m_lastWriteTime = writeTime;
            }
        }
        // 最後の読み込みの結果は、既に登録されているファイルにも反映する
        it->second.m_isLastLoadFailed = !isLoaded;
        it->second.m_listeners.emplace_back(handle, callback);
        m_handleToKey.emplace(handle, key);
#endif // K2_DEBUG

        return handle;
    }


    void HotReloadManager::Unregister(HotReloadHandle handle)
    {
        const auto keyIt = m_handleToKey.find(handle);
        if (keyIt == m_handleToKey.end())
        {
            return;
        }

        const auto fileIt = m_files.find(keyIt->second);
        if (fileIt != m_files.end())
        {
            std::vector<Listener>& listeners = fileIt->second.m_listeners;
            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(), [handle](const Listener& listener) { return listener.m_handle == handle; }),
                listeners.end()
            );

            // 関数が無くなったファイルは、調べなくていい
            if (listeners.empty())
            {
                m_files.erase(fileIt);
            }
        }

        m_handleToKey.erase(keyIt);
    }


    bool HotReloadManager::Reload([[maybe_unused]] HotReloadHandle handle)
    {
#ifdef K2_DEBUG
        const auto keyIt = m_handleToKey.find(handle);
        if (keyIt == m_handleToKey.end())
        {
            return false;
        }

        // NOTE: 関数の中でUnregister()されると、keyItが無効になるので、キーをコピーして渡す。
        const std::string key = keyIt->second;
        return ReloadFile(key);
#else
        return false;
#endif // K2_DEBUG
    }


    bool HotReloadManager::IsLastLoadFailed([[maybe_unused]] HotReloadHandle handle) const
    {
#ifdef K2_DEBUG
        const auto keyIt = m_handleToKey.find(handle);
        if (keyIt == m_handleToKey.end())
        {
            return false;
        }

        const auto fileIt = m_files.find(keyIt->second);
        if (fileIt == m_files.end())
        {
            return false;
        }

        return fileIt->second.m_isLastLoadFailed;
#else
        return false;
#endif // K2_DEBUG
    }


    void HotReloadManager::SetCheckInterval(float seconds)
    {
        m_checkInterval = (std::max)(0.0f, seconds);
    }


    std::string HotReloadManager::NormalizePath(const std::string& path)
    {
        std::wstring wide = std::filesystem::path(path).lexically_normal().generic_wstring();

        // NOTE: Windowsのパスは大文字小文字を区別しないので、小文字にそろえる。
        //       narrow文字列のままだと、Shift-JISの2バイト目(0x41〜0x5A)まで変えてしまうので、wide文字列で行う。
        for (wchar_t& c : wide)
        {
            if (c >= L'A' && c <= L'Z')
            {
                c = static_cast<wchar_t>(c - L'A' + L'a');
            }
        }
        return std::filesystem::path(wide).generic_string();
    }


    HotReloadManager& HotReloadManager::Get()
    {
        static HotReloadManager instance;
        return instance;
    }
} // namespace app
