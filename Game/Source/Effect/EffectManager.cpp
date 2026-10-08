/**
 * @file EffectManager.cpp
 * @brief エフェクトの管理をするクラス
 */
#include "stdafx.h"

#include "EffectManager.h"

#include <iterator>


namespace app
{
    namespace
    {
        /** NewGOするときのオブジェクト名 */
        constexpr const char* EFFECT_GO_NAME = "effect";

        /** 次に割り当てるハンドル */
        EffectHandle effectHandleCount = 0;
    } // namespace


    EffectManager* EffectManager::m_instance = nullptr;


    EffectManager::EffectManager()
        : m_effects()
    {
        // エフェクトの登録
        // ResistEffect()でエフェクトを読み込むには、テクスチャなどのローダーが設定されている必要がある。
        // ローダーは毎フレームの EffectEngine::BeginFrame()(K2EngineLow::BeginFrame()から呼ばれる)で設定されるが、
        // Applicationは最初のフレームの開始前に作られるので、登録の前提条件を満たすために、ここで一度だけ呼んでおく。
        EffectEngine::GetInstance()->BeginFrame();

        for (int i = 0; i < static_cast<int>(std::size(EFFECT_LIST)); ++i)
        {
            const char16_t* path = EFFECT_LIST[i].assetPath;
            // 空のパスは未登録として扱う。(プレースホルダー)
            if (path == nullptr || path[0] == u'\0')
            {
                continue;
            }
            EffectEngine::GetInstance()->ResistEffect(i, path);
        }
    }


    EffectManager::~EffectManager()
    {
        // NOTE: EffectEmitterの破棄はエンジンが行う。
        StopAllEffect();
    }


    void EffectManager::Update()
    {
        // NOTE: 再生が終わったEffectEmitterは、エンジンの更新の中で自分でDeleteGOする。
        //       実際のdeleteは次のExecuteUpdateの冒頭なので、その前にリストから外しておく。
        for (auto it = m_effects.begin(); it != m_effects.end();)
        {
            EffectEntry& entry = it->second;

            // エミッターが自分でDeleteGOした(再生が終わった)。
            if (entry.m_emitter->IsDead())
            {
                it = m_effects.erase(it);
                continue;
            }

            // 再生が終わっているが、まだエミッターがDeleteGOしていない。
            if (!entry.m_emitter->IsPlay())
            {
                DeleteGO(entry.m_emitter);
                it = m_effects.erase(it);
                continue;
            }

            // 追従先がある場合は、座標を合わせる。
            if (entry.m_followTarget != nullptr)
            {
                entry.m_emitter->SetPosition(*entry.m_followTarget + entry.m_followOffset);
            }
            ++it;
        }
    }


    EffectHandle EffectManager::PlayEffect(const EnEffectKind kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
    {
        // ハンドルが最大数になったら使えない
        // NOTE: そんなに再生するはずがない
        if (effectHandleCount == INVALID_EFFECT_HANDLE)
        {
            K2_ASSERT(false, "エフェクトの再生が多いです。\n");
            return INVALID_EFFECT_HANDLE;
        }

        // 種類が範囲外か、パスが未登録のものは再生できない。
        // NOTE: EffectEngine::LoadEffect()は、未登録だとabortするので、その前に弾く。
        const size_t index = static_cast<size_t>(kind);
        if (index >= std::size(EFFECT_LIST) || EFFECT_LIST[index].assetPath[0] == u'\0')
        {
            K2_ASSERT(false, "EFFECT_LISTに登録されていないエフェクトは再生できません。\n");
            return INVALID_EFFECT_HANDLE;
        }

        EffectEmitter* emitter = NewGO<EffectEmitter>(0, EFFECT_GO_NAME);
        emitter->Init(static_cast<int>(kind));
        emitter->SetPosition(position);
        emitter->SetRotation(rotation);
        emitter->SetScale(scale);
        emitter->Play();

        m_effects.emplace(effectHandleCount, EffectEntry(emitter));
        return effectHandleCount++;
    }


    void EffectManager::StopEffect(const EffectHandle handle)
    {
        auto it = m_effects.find(handle);
        if (it == m_effects.end())
        {
            return;
        }
        // 追従は、エミッターを止めるときに合わせて解除する。(エントリーごと消える。)
        StopEmitter(it->second.m_emitter);
        m_effects.erase(it);
    }


    void EffectManager::StopAllEffect()
    {
        for (auto& it : m_effects)
        {
            StopEmitter(it.second.m_emitter);
        }
        m_effects.clear();
    }


    EffectEmitter* EffectManager::FindEffect(const EffectHandle handle)
    {
        auto it = m_effects.find(handle);
        if (it == m_effects.end())
        {
            return nullptr;
        }
        return it->second.m_emitter;
    }


    void EffectManager::AttachEffect(const EffectHandle handle, const Vector3* target, const Vector3& offset)
    {
        auto it = m_effects.find(handle);
        if (it == m_effects.end())
        {
            return;
        }
        it->second.m_followTarget = target;
        it->second.m_followOffset = offset;
    }


    void EffectManager::StopEmitter(EffectEmitter* emitter)
    {
        emitter->Stop();
        DeleteGO(emitter);
    }


    void EffectManager::CreateInstance()
    {
        K2_ASSERT(EffectEngine::GetInstance() != nullptr, "EffectManagerはEffectEngineの生成後に作ること。");
        K2_ASSERT(m_instance == nullptr, "EffectManagerは既に生成されている。");
        if (m_instance == nullptr)
        {
            m_instance = new EffectManager();
        }
    }


    void EffectManager::DestroyInstance()
    {
        delete m_instance;
        m_instance = nullptr;
    }


    EffectManager& EffectManager::Get()
    {
        K2_ASSERT(m_instance != nullptr, "EffectManager::CreateInstance()を先に呼ぶこと。");
        return *m_instance;
    }
} // namespace app
