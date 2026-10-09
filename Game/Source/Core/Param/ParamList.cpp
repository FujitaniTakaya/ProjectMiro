/**
 * @file ParamList.cpp
 * @brief 全パラメーターの登録一覧
 */
#include "stdafx.h"

#include "ParamList.h"

#include "ParamHolder.h"
// NOTE: パラメーターの構造体と、jsonから読み込む関数のヘッダーは、ここでincludeする。


namespace app
{
    void RegisterAllParams([[maybe_unused]] ParamHolder& holder)
    {
        // 例: holder.Register<MiniMapParameter>(EnParamID::MiniMap, "Assets/parameter/ui/MiniMap.json", &LoadMiniMap);
        // 値をデバッグ画面(ParamDebugUI)で見たい場合は、4つ目の引数に表示関数を渡す。
        //     holder.Register<MiniMapParameter>(EnParamID::MiniMap, "Assets/parameter/ui/MiniMap.json", &LoadMiniMap, &DrawMiniMap);
    }
} // namespace app
