/**
 * @file SoundManager.cpp
 * @brief サウンドの管理をするクラス
 */
#include "stdafx.h"

#include "SoundManager.h"


namespace app
{
    SoundManager::SoundManager()
        : m_master(/*parent=*/nullptr)
        , m_bgm(&m_master)
        , m_se(&m_master)
        , m_voice(&m_master)
    {
        // 表の全ての音を、サウンドエンジンに登録する。(波形データは、この時点でメモリに全て読み込まれる。)
        for (const SoundInformation& info : SOUND_LIST)
        {
            // パスが空の行は未設定(プレースホルダ)なので登録しない。
            if (info.filePath == nullptr || info.filePath[0] == '\0')
            {
                continue;
            }

            // 番号は、行のidをそのまま使う。(表の並びと、EnSoundIDの並びを合わせる必要がない。)
            const int number = static_cast<int>(info.id);
            g_soundEngine->ResistWaveFileBank(number, info.filePath);

            // 開けなかったファイルは、エンジンが黙って登録を取りやめるので、ログに残す。
            if (g_soundEngine->GetWaveFileBank().FindWaveFile(number) == nullptr)
            {
                K2_LOG("サウンドの登録に失敗しました。path=%s\n", info.filePath);
            }
        }
    }


    SoundManager::~SoundManager()
    {
        // NOTE: 再生中の音は、各グループのデストラクタが止める。(子 → 親の順に破棄される。)
    }


    void SoundManager::Update()
    {
        // フェードを進める時間。
        const float deltaTime = g_gameTime->GetFrameDeltaTime();

        m_bgm.Update(deltaTime);
        m_se.Update(deltaTime);
        m_voice.Update(deltaTime);
    }


    void SoundManager::PauseAll()
    {
        m_bgm.Pause();
        m_se.Pause();
        m_voice.Pause();
    }


    void SoundManager::ResumeAll()
    {
        m_bgm.Resume();
        m_se.Resume();
        m_voice.Resume();
    }


    SoundManager* SoundManager::m_instance = nullptr;


    void SoundManager::CreateInstance()
    {
        K2_ASSERT(g_soundEngine != nullptr, "SoundManagerはg_soundEngineの生成後に作ること。");
        K2_ASSERT(m_instance == nullptr, "SoundManagerは既に生成されている。");
        if (m_instance == nullptr)
        {
            m_instance = new SoundManager();
        }
    }


    void SoundManager::DestroyInstance()
    {
        delete m_instance;
        m_instance = nullptr;
    }


    SoundManager& SoundManager::Get()
    {
        K2_ASSERT(m_instance != nullptr, "SoundManager::CreateInstance()を先に呼ぶこと。");
        return *m_instance;
    }
} // namespace app
