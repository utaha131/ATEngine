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
	// float2 uv_scale = 1.0f / float2(1280.0f, 720.0f);
	// float3 color = float3(0.0f, 0.0f, 0.0f);

	// for (int i = -1; i <= 1; ++i) {
	// 	for (int j = -1; j <= 1; ++j) {
	// 		float2 uv_offset = uv_scale * float2(j, i);
	// 		color += g_InputTexture.Sample(g_Sampler, input.UV.xy + uv_offset).rgb;
	// 	}
	// }

	// color /= 9.0f;

	float3 color =  g_InputTexture.Sample(g_Sampler, input.UV.xy).rgb;
	float lum = dot(color, float3(0.299, 0.587, 0.114));
	float r = lum / (lum + 1.0f);
	float3 ldr = color * (r / lum);
	//float3 ldr = color.rgb / (color.rgb + float3(1.0f, 1.0f, 1.0f));
	//float3 ldr = ACESFilm(color.rgb);
	float gamma = 2.2f;
	return float4(pow(ldr.rgb, 1.0f / gamma), 1.0f);
}