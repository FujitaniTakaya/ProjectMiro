/**
 * @file ParamList.h
 * @brief パラメーターのIDと、全パラメーターの登録一覧
 * @details IDと、IDごとのjsonファイル・読み込み方の登録(ParamList.cpp)は、ここが入り口。
 *          NOTE: このヘッダーには、パラメーターの構造体のヘッダーをincludeしないこと。
 *                ParamHolderを使う全てのファイルが、全てのパラメーターの構造体に依存してしまう。
 *                構造体のヘッダーは、登録する側のParamList.cppでincludeする。
 */
#pragma once
#include <cstdint>


namespace app
{
    class ParamHolder;


    /**
     * @brief パラメーターのID
     * @details ParamHolderに登録するパラメーターの種類。RegisterAllParams()で、全てのIDにjsonファイルと読み込み方を登録すること。
     *          パラメーターを追加するときは、Maxの前に足す。
     */
    enum class EnParamID : uint8_t
    {
        Max,
        None = Max
    };


    /**
     * @brief 全てのパラメーターを、ParamHolderに登録する
     * @details ParamHolderのコンストラクタから呼ばれる。EnParamIDの全てのIDを登録すること。(登録漏れは、ParamHolder生成時にK2_ASSERTで分かる。)
     *          パラメーターを追加するときは、次の4つを行う。
     *          1. EnParamIDにIDを足す。
     *          2. パラメーターの構造体を作る。(普通の構造体でよい。配列のjsonなら、std::vectorにする。)
     *          3. jsonから構造体に読み込む関数を作る。(ParamLoaderを使う。構造体は、デフォルト構築された状態で渡される。)
     *          4. ParamList.cppに、holder.Register<構造体>(ID, jsonのパス, 読み込む関数); を1行足す。
     *          デバッグ画面(ParamDebugUI)で値を見たい場合は、Register()の4つ目の引数に表示関数を渡す。(省略可)
     *          表示関数は void(const 構造体&) で、ImGuiの呼び出しは #ifdef BALLOON_IMGUI_ENABLED で囲むこと。
     * @param holder 登録先
     */
    void RegisterAllParams(ParamHolder& holder);
} // namespace app
