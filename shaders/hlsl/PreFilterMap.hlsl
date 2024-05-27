#include "Cube.hlsli"
#include "IBL.hlsli"

[[vk::binding(0, 0)]] cbuffer cbPerPass : register(b0, space0) {
    float4x4 MV_Matrix;
    float Roughness;
    uint Mip_Slice;
}

[[vk::binding(1, 0)]] TextureCube enviroment_map : register(t0, space0);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0, space0);

struct PixelIn {
    float4 Position : SV_POSITION;
    float3 UV : TEXCOORD;
};

PixelIn VS(uint VertexID : SV_VertexID) {
    PixelIn output;
    float4 position = Cube[VertexID];
    output.UV = position.xyz;
    position = mul(MV_Matrix, position);
    output.Position = position.xyzw;
    return output;
}

float4 PS(PixelIn input) : SV_Target {
  float3 R = normalize(input.UV);
  float3 N = R;
  float3 V = R;
  float3 PrefilteredColor = 0;
  const uint NumSamples = 64;
  float TotalWeight = 0.0f;
  for(uint i = 0; i < NumSamples; i++) {
    float2 Xi = Hammersley(i, NumSamples);
    float3 H = ImportanceSampleGGX(Xi, Roughness, N);
    float3 L = 2 * dot(V, H) * H - V;
    float NoL = saturate(dot( N, L ));
    if( NoL > 0 ) {
      PrefilteredColor += enviroment_map.SampleLevel(g_Sampler, L, Mip_Slice).rgb * NoL;
      TotalWeight += NoL;
    }
  }
  return float4(PrefilteredColor / TotalWeight, 1.0f);
}