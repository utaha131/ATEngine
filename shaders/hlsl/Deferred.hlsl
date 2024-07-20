#include "Lighting.hlsli"
#include "BRDF.hlsli"
#include "Shadow.hlsli"
#include "Quad.hlsli"
#define LIGHT_SOURCE_COUNT 5

struct PixelIn {
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

[[vk::binding(0, 0)]] cbuffer cbPerObject : register(b0) {
	Light Lights[LIGHT_SOURCE_COUNT];
	float4x4 InverseProjectionMatrix;
	float4x4 InverseViewMatrix;
};

[[vk::binding(1, 0)]] Texture2D g_BaseColorTexture : register(t0);
[[vk::binding(2, 0)]] Texture2D g_NormalTexture : register(t1);
[[vk::binding(3, 0)]] Texture2D g_SurfaceTexture : register(t2);
[[vk::binding(4, 0)]] Texture2D g_DepthTexture : register(t3);
[[vk::binding(5, 0)]] Texture2DArray g_DirectionalShadowMaps[LIGHT_SOURCE_COUNT] : register(t4);
[[vk::binding(10, 0)]] TextureCube g_PointShadowMaps[LIGHT_SOURCE_COUNT] : register(t9);
[[vk::binding(15, 0)]] SamplerState g_Sampler : register(s0);
[[vk::binding(16, 0)]] SamplerState g_ShadowSampler : register(s1);

PixelIn VS(uint VertexID : SV_VertexID) {
	PixelIn output;
	output.Position = float4(Quad[VertexID].xyz, 1.0f);
	output.UV = Quad_UV[VertexID];
	return output;
}

float4 PS(PixelIn input): SV_Target {
	BSDFParameters parameters;
	float depth = g_DepthTexture.Sample(g_Sampler, input.UV).r;
	float2 coords = float2(input.UV.x, 1.0f - input.UV.y) * 2.0f - 1.0f;
	float4 view_space_fragment_position = mul(InverseProjectionMatrix, float4(coords.xy, depth, 1.0f)).xyzw;
	view_space_fragment_position.xyz /= view_space_fragment_position.w;
	float4 Surface = g_SurfaceTexture.Sample(g_Sampler, input.UV).rgba;
	parameters.V = normalize(-view_space_fragment_position.xyz);
	parameters.N = normalize(g_NormalTexture.Sample(g_Sampler, input.UV).xyz * 2.0f - 1.0f);
	parameters.R = normalize(reflect(-parameters.V, parameters.N));
	parameters.NdotV = saturate(dot(parameters.N, parameters.V));
	float4 base_color = g_BaseColorTexture.Sample(g_Sampler, input.UV).rgba;
	if (base_color.a  == 0.0f) {
	 	discard;
	}
	parameters.DiffuseColor = float4(base_color.rgb * (1.0f - Surface.b), base_color.a);
	parameters.Metalness = Surface.b;
	parameters.Roughness = Surface.g;
	parameters.SpecularColor = float3(lerp(F0_DIELECTRICS, base_color.rgb, Surface.bbb).rgb);
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float4 world_space_fragment_position = mul(InverseViewMatrix, float4(view_space_fragment_position.xyz, 1.0f));
	world_space_fragment_position.xyz /= world_space_fragment_position.w;

	for(int i = 0; i < LIGHT_SOURCE_COUNT; ++i) {
		float shadow = 1.0f;
		float attenuation = 1.0f;
		if (Lights[i].Type == 1) { //Directional Light
			parameters.L = normalize(Lights[i].PositionOrDirection.xyz);
			shadow = CascadedShadowMapFactor(view_space_fragment_position.xyz, world_space_fragment_position.xyz, Lights[i].Matrices, g_DirectionalShadowMaps[i], g_ShadowSampler);
		} else if (Lights[i].Type == 2) {
			float d = distance(Lights[i].PositionOrDirection.xyz, view_space_fragment_position.xyz);
			attenuation = saturate(1.0f - pow(d / Lights[i].Radius, 4.0f));
			attenuation =  attenuation * attenuation / (d * d + 1.0f);
			parameters.L = normalize(Lights[i].PositionOrDirection.xyz - view_space_fragment_position.xyz);
			shadow = OmnidirectionalShadowMapFactor(world_space_fragment_position.xyz, Lights[i].PositionOrDirection.xyz, InverseViewMatrix, g_PointShadowMaps[i], g_ShadowSampler, Lights[i].Radius);
		}
		parameters.H = normalize(parameters.L + parameters.V);
		parameters.NdotL = saturate(dot(parameters.N, parameters.L));
		parameters.LdotH = saturate(dot(parameters.L, parameters.H));
		parameters.NdotH = saturate(dot(parameters.N, parameters.H));
		parameters.VdotH = saturate(dot(parameters.V, parameters.H));
		color += BRDF(parameters) * Lights[i].Intensity * Lights[i].Color * attenuation * parameters.NdotL * shadow;
	}
	
	return float4(color.rgb,  parameters.DiffuseColor.a);
}
 	