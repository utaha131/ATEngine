#include "Quad.hlsli"

struct PixelIn {
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

[[vk::binding(0, 0)]] cbuffer PassData : register(b0) {
	float2 Size;
};
[[vk::binding(1, 0)]] Texture2D g_SSAO : register(t0);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0);

PixelIn VS(uint VertexID : SV_VertexID) {
	PixelIn output;
	output.Position = float4(Quad[VertexID].xyz, 1.0f);
	output.UV = Quad_UV[VertexID];
	return output;
}

float PS(PixelIn input) : SV_Target {
	float2 ssao_texel_size = 1.0f / Size;
	float result = 0.0f;
	for (int x = -2; x < 2; ++x) {
		for (int y = -2; y < 2; ++y) {
			float2 offset = input.UV.xy + float2(float(x), float(y)) * ssao_texel_size;
			result += g_SSAO.Sample(g_Sampler, offset.xy).r;
		}
	}
	return result / (4.0f * 4.0f);
}