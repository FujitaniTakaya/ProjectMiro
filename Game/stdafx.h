#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "k2EngineLowPreCompile.h"
using namespace nsK2EngineLow;

#include "BalloonEngine.h"
using namespace nsBalloonEngine;

// NOTE: 標準ヘッダーは、エンジンのヘッダーより後に書く。
//       k2EngineLowPreCompile.h が先頭で _CRT_NON_CONFORMING_SWPRINTFS を定義するので、先に標準ヘッダーを読むと vswprintf がエラーになる。
#include <optional>
