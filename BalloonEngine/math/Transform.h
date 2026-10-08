/**
 * @file Transform.h
 * @brief 変換情報を保持するクラスの宣言
 */
#pragma once


namespace nsBalloonEngine
{
    /**
     * @brief 変換情報を保持するクラス
     */
    class Transform
    {
    public:
        /**
         * @brief コンストラクタ
         * @details 座標は(0, 0, 0)、回転は単位クォータニオン、拡大は(1, 1, 1)で初期化する。
         */
        Transform();


    public:
        /** 座標 */
        Vector3 m_position;
        /** 回転 */
        Quaternion m_rotation;
        /**	拡大 */
        Vector3 m_scale;
    };
} // namespace nsBalloonEngine