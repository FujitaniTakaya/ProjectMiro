/**
 * @file Transform.cpp
 * @brief 変換情報を保持するクラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "Transform.h"


namespace nsBalloonEngine
{
    // NOTE: 初期値は、リテラルで初期化している。
    //       Vector3::Zeroなど、他のファイルのstatic変数から初期化すると、
    //       他のファイルのstatic変数の初期化中に生成された場合に、初期化の順番によってはゼロのままコピーされてしまうため。
    Transform::Transform()
        : m_position(0.0f, 0.0f, 0.0f)
        , m_rotation(0.0f, 0.0f, 0.0f, 1.0f)
        , m_scale(1.0f, 1.0f, 1.0f)
    {}
} // namespace nsBalloonEngine