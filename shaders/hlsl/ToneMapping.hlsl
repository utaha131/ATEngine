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

[[vk::binding(0, 0)]] Texture2D g_InputTexture : register(t0);
[[vk::binding(1, 0)]] SamplerState g_Sampler : register(s0);

float3 ACESFilm(float3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}


float4 PS(PixelIn input) : SV_Target {
	float3 color =  g_InputTexture.Sample(g_Sampler, input.UV.xy).rgb;
	//float3 ldr = color.rgb / (color.rgb + float3(1.0f, 1.0f, 1.0f));
	float3 ldr = ACESFilm(color.rgb);
	return float4(ldr.rgb, 1.0f);
}