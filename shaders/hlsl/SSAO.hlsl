#include "Quad.hlsli"
//Shader Parameters.
[[vk::binding(0, 0)]] cbuffer Matrices : register(b0) {
	float4x4 InverseProjectionMatrix;
	float4x4 ProjectionMatrix;
	float4 Kernel[64];
	float2 Size;
};

[[vk::binding(1, 0)]] Texture2D g_DepthTexture : register(t0);
[[vk::binding(2, 0)]] Texture2D g_NormalTexture : register(t1);
[[vk::binding(3, 0)]] Texture2D g_NoiseTexture : register(t2);
[[vk::binding(4, 0)]] SamplerState g_Sampler : register(s0);


//Shader IO.

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

float4 PS(PixelIn input) : SV_Target {
	float depth = g_DepthTexture.Sample(g_Sampler, input.UV).r;
	float2 coords = float2(input.UV.x, 1.0f - input.UV.y) * 2.0f - 1.0f;
	float4 view_position = mul(InverseProjectionMatrix, float4(coords.xy, depth, 1.0f));
	view_position.xyz = view_position.xyz / view_position.w;
	float3 normal = normalize(g_NormalTexture.Sample(g_Sampler, input.UV).xyz * 2.0f - 1.0f);
	float2 scale = float2(Size.x / 4.0f, Size.y / 4.0f);
	float3 random_vector = normalize(g_NoiseTexture.Sample(g_Sampler, input.UV * scale).xyz * 2.0f - 1.0f);
	float3 tangent = normalize(random_vector - normal * dot(random_vector, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = transpose(float3x3(tangent, bitangent, normal));

	float3 fragment_position = view_position.xyz;

	float occlusion = 0.0f;
	float radius = 0.5f;
	for (int i = 0; i < 64; ++i) {
		float3 sample_position = mul((float3x3)TBN, Kernel[i].xyz);
		sample_position = fragment_position + sample_position * radius;

		float4 offset = float4(sample_position, 1.0f);
		offset = mul(ProjectionMatrix, offset);
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5f + 0.5f;
		offset.y = 1.0f - offset.y;

		float sample_depth = g_DepthTexture.Sample(g_Sampler, offset.xy).r;
		float4 sample_depth_position = mul(InverseProjectionMatrix, float4(0.0f, 0.0f, sample_depth, 1.0f));
		sample_depth = sample_depth_position.z / sample_depth_position.w;

		float bias = 0.03f;

		float range_check = smoothstep(0.0, 1.0, radius / abs(fragment_position.z - sample_depth));
		occlusion += (sample_depth >= sample_position.z + bias ? 1.0 : 0.0) * range_check;  
	}

	occlusion = 1.0f - (occlusion / 64.0f);
	return occlusion;
}