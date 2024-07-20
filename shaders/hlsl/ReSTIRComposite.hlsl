#include "Quad.hlsli"

struct PixelIn {
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

PixelIn VS(uint VertexID : SV_VertexID) {
	PixelIn output;
	output.Position = float4(Quad[VertexID].xyz, 1.0f);
	output.UV = Quad_UV[VertexID];
	return output;
}

[[vk::binding(0, 0)]] Texture2D gDirectIllumination : register(t0);
[[vk::binding(1, 0)]] Texture2D gGlobalIllumination : register(t1);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0);



float4 PS(PixelIn input) : SV_Target {
	return float4(gDirectIllumination.Sample(g_Sampler, input.UV).rgb + gGlobalIllumination.Sample(g_Sampler, input.UV).rgb, 1.0f);
}