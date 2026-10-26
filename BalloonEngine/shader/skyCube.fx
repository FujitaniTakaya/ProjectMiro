/**
 * @brief   空(スカイキューブ)を描くシェーダー
 * @details デファードライティングの後に、全画面のスプライトとして描く。
 *          G-Buffer に何も描かれていない画素(深度が0のまま)にだけ、キューブマップの色を書く。
 *          カメラが動いても空は動かないように、キューブマップは画素ごとの視線の方向だけで引く。
 *
 *          スプライトのテクスチャ: t0 = スカイキューブ(TextureCube)、t1 = G-Buffer の深度。
 *          拡張定数バッファ(b1)は、デファードライティングと同じ LightingCB(視点と mViewProjInv を使う)。
 *          明るさは、スプライトの乗算カラー(mulColor)で渡す。
 */
cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
}


#include "common/Lighting.hlsli"


TextureCube<float4> g_skyCube : register(t0);
Texture2D<float> g_depth : register(t1);


sampler g_sampler : register(s0);


/** αに入れる、カメラからの距離。デファードライティングはαに距離を入れる(DoFが使う)ので、空は最も遠い値にする */
static const float SKY_DISTANCE = 10000.0f;


struct VSIn{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSIn{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};


PSIn VSMain(VSIn In)
{
    PSIn psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}


/*!
 *@brief        UV座標と深度値からワールド座標を計算する
 *@param[in]    uv：uv座標
 *@param[in]    zInProjectionSpace：射影空間の深度値
 *@param[in]    mViewProjInv：ビュープロジェクション行列の逆行列
 */
float3 CalcWorldPosFromUVZ(float2 uv, float zInProjectionSpace, float4x4 mViewProjInv)
{
    float3 screenPos;
    screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
    screenPos.z = zInProjectionSpace;

    float4 worldPos = mul(mViewProjInv, float4(screenPos, 1.0f));
    worldPos.xyz /= worldPos.w;
    return worldPos.xyz;
}


float4 PSMain(PSIn In) : SV_Target0
{
    // G-Buffer に何かが描かれた画素は、デファードライティングの結果をそのまま残す。
    // 深度のクリア値は0で、描かれた画素の深度は0より大きい。
    // NOTE: 画面と G-Buffer は同じ大きさなので、フィルターせずに画素の位置で読む。
    const float depth = g_depth.Load(int3((int2)In.pos.xy, 0));
    if (depth > 0.0f)
    {
        discard;
    }

    // ファークリップ面の点に向かう方向が、その画素の視線の方向
    const float3 farPos = CalcWorldPosFromUVZ(In.uv, 1.0f, mViewProjInv);
    const float3 viewDir = normalize(farPos - eyePos);

    const float3 skyColor = g_skyCube.SampleLevel(g_sampler, viewDir, 0.0f).rgb * mulColor.rgb;
    return float4(skyColor, SKY_DISTANCE);
}
