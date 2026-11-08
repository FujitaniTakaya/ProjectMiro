/**
 * @file sceneBlur.fx
 * @brief 画面全体を、ぼかした画像に置き換えていくシェーダー
 * @details dof.fx と同じく、元の画像(t0)とぼかした画像(t1)を混ぜる。
 *          dof.fx は混ぜる割合をピクセルごとの距離で決めるが、こちらは全画面で同じ割合(定数バッファの blendRate)にする。
 */



cbuffer SceneBlurCB : register(b1)
{
    // ぼかした画像を混ぜる割合。0で元の画像のまま、1でぼかした画像だけ
    float blendRate;
    // 4バイトアラインメントのためのパディング
    float padding[3];
};


#include "BloomVSCommon.hlsli"


Texture2D<float4> g_mainTexture : register(t0);
Texture2D<float4> g_bokeTexture : register(t1);
sampler g_sampler : register(s0);


float4 PSMain(PSInput In) : SV_Target0
{
    // NOTE: mainRT のアルファには、カメラからの距離が入っている。画面には出さないので、アルファは必ず1にする。
    float3 sharp = g_mainTexture.Sample(g_sampler, In.uv).xyz;
    float3 boke = g_bokeTexture.Sample(g_sampler, In.uv).xyz;
    return float4(lerp(sharp, boke, blendRate), 1.0f);
}
