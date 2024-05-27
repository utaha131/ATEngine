#include "Cube.hlsli"

struct PixelIn {
    float4 Position : SV_POSITION;
    float3 UV : TEXCOORD;
};

[[vk::binding(0, 0)]] cbuffer Constants : register(b0, space0) {
    float4x4 ModelViewMatrix;
};

[[vk::binding(1, 0)]] TextureCube g_SkyboxTexture : register(t0, space0);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0, space0);

PixelIn VS(uint VertexID : SV_VertexID) {
    PixelIn output;
    float4 position = Cube[VertexID];
    output.UV = position.xyz;
    output.Position = mul(ModelViewMatrix, position).xyww;
    return output;
}

float4 PS(PixelIn input) : SV_Target {
    float3 color = g_SkyboxTexture.Sample(g_Sampler, normalize(input.UV)).rgb;
    float3 inverse_reinhard = pow(color, 2.2f);//((1.0f - color) + 0.0001f);
    return float4(inverse_reinhard, 1.0f);
}