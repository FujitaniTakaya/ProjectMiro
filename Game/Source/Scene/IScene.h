/**
 * @file IScene.h
 * @brief シーンの基底クラス
 * @details 新しいシーンは、IScene を継承して appScene(クラス名) を書き、SceneManager のコンストラクタで AddSceneMap<T>() を呼ぶ。
 *          シーンの遷移は、RequesutScene() が true を返したときに SceneManager が行う。
 */
#pragma once
#include "Source/Util/CRC32.h"


 /**
  * シーンのIDを返す関数 ID() を作るマクロ。
  * Hash32は文字列を数値に変換するもの。数値に変換するときに別の文字列でも同じ数値になるケースがあるが、被りづらいようなアルゴリズムを使っている。
  * @note constexpr変数に受けることで、ハッシュ値を必ずコンパイル時に計算する
  *       (戻り値を直接 Hash32() にすると、実行時に毎回CRC32を計算してしまう)
  */
#define appScene(name)\
public:\
 static constexpr uint32_t ID() { constexpr uint32_t id = Hash32(#name); return id; }


namespace app
{
    /** 無効なシーンID */
    static constexpr uint32_t INVALID_SCENE_ID = 0xffffffff;


    /**
     * @brief シーンの基底クラス
     */
    class IScene : public Noncopyable
    {
    public:
        IScene() {}
        virtual ~IScene() {}


        /**
         * @brief シーンが生成された直後に1度だけ呼ばれる
         * @return 成功したらtrue
         */
        virtual bool Start() = 0;
        /** @brief 更新 */
        virtual void Update() = 0;
        /**
         * @brief 描画
         * @param rc レンダーコンテキスト
         */
        virtual void Render(RenderContext& rc) = 0;
        /** @brief 通常の更新が止まっているとき(フェードアウト中など)の更新 */
        virtual void PauseUpdate() = 0;


        /**
         * @brief シーンのロードが完了したかどうかを返す
         * @details false を返している間は、SceneManager が暗転したまま待つ。
         * @return 完了していればtrue
         */
        virtual bool IsLoaded() const { return true; }


        /**
         * @brief シーンの遷移を要求する
         * @param id[out] 次のシーンのID
         * @param waitTime[out] 暗転・明転にかける時間(秒)
         * @return 遷移したいときtrue
         */
        virtual bool RequesutScene(uint32_t& id, float& waitTime) = 0;
    };
}
