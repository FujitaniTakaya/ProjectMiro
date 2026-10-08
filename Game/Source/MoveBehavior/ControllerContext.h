/**
 * @file ControllerContext.h
 * @brief Controllerが操作対象について知ることができる情報
 * @details 操作対象のキャラクターが、毎フレーム作ってControllerに渡す読み取り専用の情報。
 *          Controllerは操作対象の具体的な型を知らなくてよい。(どのキャラクターにも付けられる)
 *          AIの目標やホーム位置などの、AI固有の情報はここに入れない。AI自身が持つ。
 */
#pragma once


namespace app
{
    /**
     * @brief Controllerが操作対象について知ることができる情報
     */
    struct ControllerContext
    {
        ControllerContext()
            : position(Vector3::Zero)
            , forward(Vector3::Front)
            , deltaTime(0.0f)
        {
        }


        /** 操作対象のワールド座標(足元) */
        Vector3 position;
        /** 操作対象が向いているワールド方向(yは無視される。長さは問わない) */
        Vector3 forward;
        /** 前フレームからの経過時間(秒)。ControllerSlot が埋めるので、持ち主は設定しなくてよい */
        float deltaTime;
    };
}
