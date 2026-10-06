/**
 * @file EffectManager.h
 * @brief エフェクトの管理をするクラス
 * @details エフェクトの登録・再生・停止・追従・再生が終わったエフェクトの回収を行う。シングルトン。
 *          エフェクトの再生は、k2EngineLowのEffectEmitterをNewGOして行う。
 *          例: const EffectHandle handle = EffectManager::Get().PlayEffect(kind, position, rotation, scale);
 *              EffectManager::Get().StopEffect(handle);
 *          NOTE: エフェクトの描画(EffectEngine::Draw())は、BalloonEngine::RenderingEngineが行う。
 *                カリングは行わない。
 */
#pragma once
#include <map>

#include "EffectHandle.h"


namespace app
{
    /**
     * @brief エフェクトを管理するクラス
     */
    class EffectManager : public Noncopyable
    {
    private:
        /**
         * @brief 再生中のエフェクトの情報
         */
        struct EffectEntry
        {
            /** エフェクトエミッター。NewGOで作られるのでメモリの管理はエンジンが行う。 */
            EffectEmitter* m_emitter = nullptr;
            /** 追従先の座標へのポインタ。nullptrなら追従しない。 */
            const Vector3* m_followTarget = nullptr;
            /** 追従先からのオフセット */
            Vector3 m_followOffset = Vector3::Zero;
        };


        /** ハンドルと再生中のエフェクトの対応 */
        using EffectList = std::map<EffectHandle, EffectEntry>;


    private:
        EffectManager();
        ~EffectManager();


    public:
        /**
         * @brief 更新処理
         * @details 再生が終わったエフェクトを、エンジンに破棄される前に保持しているリストから外す。
         *          追従するエフェクトの座標も更新する。
         *          エンジンの更新(g_engine->ExecuteUpdate())の前に、毎フレーム呼ぶこと。(Application::PreUpdate()から呼ぶ。)
         */
        void Update();


        /**
         * @brief エフェクトを再生する
         * @param kind 種類
         * @param position 座標
         * @param rotation 回転
         * @param scale 拡大率
         * @return ハンドル。再生できなかった場合はINVALID_EFFECT_HANDLE。
         */
        EffectHandle PlayEffect(const EnEffectKind kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale);


        /**
         * @brief エフェクトを停止する
         * @details 追従も解除される。
         * @param handle 停止するエフェクトのハンドル
         */
        void StopEffect(const EffectHandle handle);


        /**
         * @brief 再生している全てのエフェクトを停止する
         */
        void StopAllEffect();


        /**
         * @brief ハンドルからエフェクトエミッターを取得する
         * @details 再生が終わったエフェクトは、エンジンの更新(ExecuteUpdate)の中で自分で破棄を依頼するので、
         *          エンジンの更新の後、次のUpdate()の前までは、再生が終わった(IsPlay()がfalseの)エミッターが返ることがある。
         *          取得したポインタは、保持せずにその場で使うこと。
         * @param handle エフェクトのハンドル
         * @return エフェクトエミッター。存在しない場合はnullptr。
         */
        EffectEmitter* FindEffect(const EffectHandle handle);


        /**
         * @brief エフェクトを座標に追従させる
         * @details 毎フレーム「*target + offset」の座標に、エフェクトの座標を更新する。
         *          StopEffect()を呼ぶと追従が解除される。
         *          targetは、エフェクトの再生中に解放してはいけない。(先にStopEffect()を呼ぶこと。)
         * @param handle 追従させるエフェクトのハンドル
         * @param target 追従先の座標へのポインタ
         * @param offset 追従先からのオフセット
         */
        void AttachEffect(const EffectHandle handle, const Vector3* target, const Vector3& offset = Vector3::Zero);


    private:
        /**
         * @brief エフェクトエミッターを停止して、エンジンに破棄を依頼する
         * @param emitter エフェクトエミッター
         */
        void StopEmitter(EffectEmitter* emitter);


    private:
        /** 再生中のエフェクト */
        EffectList m_effects;


    public:
        /**
         * @brief インスタンスを生成する
         * @details EffectEngineの生成後(g_engineの初期化後)に呼ぶこと。
         */
        static void CreateInstance();


        /**
         * @brief インスタンスを破棄する
         * @details 再生中のエフェクトを止めるので、エンジン(g_engine)の破棄前に呼ぶこと。
         */
        static void DestroyInstance();


        /**
         * @brief インスタンスを取得する
         * @return インスタンス。CreateInstance()を呼ぶ前に取得してはいけない。
         */
        static EffectManager& Get();


    private:
        /** 唯一のインスタンス */
        static EffectManager* m_instance;
    };
} // namespace app
