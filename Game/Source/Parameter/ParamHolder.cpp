/**
 * @file ParamHolder.cpp
 * @brief パラメーターを保持して、IDで取り出せるようにするクラス
 */
#include "stdafx.h"
#include "ParamHolder.h"

#include "Source/Parameter/ParamList.h"


namespace app
{
    ParamHolder* ParamHolder::m_instance = nullptr;


    ParamHolder::ParamHolder()
        : m_entries()
    {
        RegisterAllParams(*this);

        // 全てのIDが登録されているか確認する
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            K2_ASSERT(m_entries[i] != nullptr, "ParamList.cppに登録されていないIDがある。");
        }
    }


    bool ParamHolder::IsLoaded(EnParamID id) const
    {
        const IEntry* entry = FindEntry(id);
        return entry != nullptr && entry->IsLoaded();
    }


    bool ParamHolder::GetDebugInfo(EnParamID id, ParamDebugInfo& outInfo) const
    {
        const IEntry* entry = FindEntry(id);
        if (entry == nullptr) return false;

        outInfo = entry->GetDebugInfo();
        return true;
    }


    void ParamHolder::DrawValue(EnParamID id) const
    {
        const IEntry* entry = FindEntry(id);
        if (entry == nullptr) return;

        entry->DrawValue();
    }


    bool ParamHolder::Reload(EnParamID id)
    {
        IEntry* entry = FindEntry(id);
        return entry != nullptr && entry->Reload();
    }


    const ParamHolder::IEntry* ParamHolder::FindEntry(EnParamID id) const
    {
        const size_t index = static_cast<size_t>(id);
        if (index >= m_entries.size()) return nullptr;

        return m_entries[index].get();
    }


    ParamHolder::IEntry* ParamHolder::FindEntry(EnParamID id)
    {
        // NOTE: constの版と同じ処理なので、そちらに任せる。
        return const_cast<IEntry*>(static_cast<const ParamHolder*>(this)->FindEntry(id));
    }


    void ParamHolder::CreateInstance()
    {
        K2_ASSERT(m_instance == nullptr, "ParamHolderは既に生成されている。");
        if (m_instance == nullptr)
        {
            m_instance = new ParamHolder();
        }
    }


    void ParamHolder::DestroyInstance()
    {
        delete m_instance;
        m_instance = nullptr;
    }


    ParamHolder& ParamHolder::Get()
    {
        K2_ASSERT(m_instance != nullptr, "ParamHolder::CreateInstance()を先に呼ぶこと。");
        return *m_instance;
    }
} // namespace app
