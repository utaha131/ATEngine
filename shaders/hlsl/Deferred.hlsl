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
	Light lights[LIGHT_SOURCE_COUNT];
	float4x4 Inverse_Projection_Matrix;
	float4x4 Inverse_View_Matrix;
	bool Use_Enviroment_Map;
};

[[vk::binding(1, 0)]] Texture2D g_BaseColor: register(t0);
[[vk::binding(2, 0)]] Texture2D g_Normal : register(t1);
[[vk::binding(3, 0)]] Texture2D g_Surface : register(t2);
[[vk::binding(4, 0)]] Texture2D g_Depth : register(t3);
[[vk::binding(5, 0)]] Texture2D g_SSAO : register(t4);
[[vk::binding(6, 0)]] Texture2DArray g_DirectionalShadowMaps[LIGHT_SOURCE_COUNT] : register(t5);
[[vk::binding(11, 0)]] TextureCube g_PointShadowMaps[LIGHT_SOURCE_COUNT] : register(t10);
[[vk::binding(16, 0)]] TextureCube enviroment_map : register(t15);
[[vk::binding(17, 0)]] TextureCube prefiltered_map : register(t16);
[[vk::binding(18, 0)]] Texture2D env_brdf : register(t17);
[[vk::binding(19, 0)]] SamplerState g_Sampler : register(s0);
[[vk::binding(20, 0)]] SamplerState g_ShadowSampler : register(s1);

PixelIn VS(uint VertexID : SV_VertexID) {
	PixelIn output;
	output.Position = float4(Quad[VertexID].xyz, 1.0f);
	output.UV = Quad_UV[VertexID];
	return output;
}

static const float3 F0_DIELECTRICS = float3(0.04f, 0.04f, 0.04f);

float4 PS(PixelIn input): SV_Target {
	BSDF_Parameters parameters;
	float depth = g_Depth.Sample(g_Sampler, input.UV).r;
	float2 coords = float2(input.UV.x, 1.0f - input.UV.y) * 2.0f - 1.0f;
	float4 Fragment_Position = mul(Inverse_Projection_Matrix, float4(coords.xy, depth, 1.0f)).xyzw;
	Fragment_Position.xyz /= Fragment_Position.w;
	float4 Surface = g_Surface.Sample(g_Sampler, input.UV).rgba;
	parameters.V = normalize(-Fragment_Position.xyz);
	parameters.N = normalize(g_Normal.Sample(g_Sampler, input.UV).xyz * 2.0f - 1.0f);
	parameters.R = normalize(reflect(-parameters.V, parameters.N));
	parameters.NdotV = saturate(dot(parameters.N, parameters.V));
	float4 base_color = g_BaseColor.Sample(g_Sampler, input.UV).rgba;
	if (base_color.a  == 0.0f) {
	 	discard;
	}
	parameters.Diffuse = float4(base_color.rgb * (1.0f - Surface.b), base_color.a);
	parameters.Metalness = Surface.b;
	parameters.Roughness = Surface.g;
	parameters.Specular_Color = float3(lerp(F0_DIELECTRICS, base_color.rgb, Surface.bbb).rgb);
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float4 Fragment_World_Position = mul(Inverse_View_Matrix, float4(Fragment_Position.xyz, 1.0f));
	Fragment_World_Position.xyz /= Fragment_World_Position.w;

	for(int i = 0; i < LIGHT_SOURCE_COUNT; ++i) {
		float shadow = 1.0f;
		float attenuation = 1.0f;
		if (lights[i].Type == 1) { //Directional Light
			parameters.L = normalize(lights[i].Position_Or_Direction.rgb);
			shadow = CascadedShadowMapFactor(Fragment_Position.xyz, Fragment_World_Position.xyz, lights[i].Matrices, g_DirectionalShadowMaps[i], g_ShadowSampler);
		} else if (lights[i].Type == 2) {
			float d = distance(lights[i].Position_Or_Direction.rgb, Fragment_Position);
			attenuation = saturate(1.0f - pow(d / lights[i].Radius, 4.0f));
			attenuation =  attenuation * attenuation / (d * d + 1.0f);
			parameters.L = normalize(lights[i].Position_Or_Direction.rgb - Fragment_Position);
			shadow = OmnidirectionalShadowMapFactor(Fragment_World_Position.xyz, lights[i].Position_Or_Direction.xyz, Inverse_View_Matrix, g_PointShadowMaps[i], g_ShadowSampler, lights[i].Radius);
		}
		parameters.H = normalize(parameters.L + parameters.V);
		parameters.NdotL = saturate(dot(parameters.N, parameters.L));
		parameters.LdotH = saturate(dot(parameters.L, parameters.H));
		parameters.NdotH = saturate(dot(parameters.N, parameters.H));
		parameters.VdotH = saturate(dot(parameters.V, parameters.H));
		color += BRDF(parameters) * lights[i].Intensity * attenuation * parameters.NdotL * shadow;
	}
	float3 ambient = 0.0f;
	
	if (Use_Enviroment_Map) {
		float3 irradiance = enviroment_map.Sample(g_ShadowSampler, parameters.N).rgb;
		ambient = parameters.Diffuse.rgb * 0.3f;//irradiance;
		float3 N = normalize(mul(Inverse_View_Matrix, parameters.N));
		float3 V = normalize(mul(Inverse_View_Matrix, -parameters.V));
		float3 R = normalize(reflect(V, N));
		float3 prefiltered_color = prefiltered_map.SampleLevel(g_ShadowSampler, R, (int)((parameters.Roughness) * 8.0f));
		float2 brdf = env_brdf.Sample(g_ShadowSampler, float2(saturate(dot(N, V)), 1.0f - parameters.Roughness));
		float3 specular = prefiltered_color * (parameters.Specular_Color * brdf.x + saturate( 50.0 * parameters.Specular_Color.g ) * brdf.y) * 0.4f;

		ambient += specular;
		color += g_SSAO.Sample(g_Sampler, input.UV).r * ambient;
	} else {
		ambient = parameters.Diffuse.rgb * 0.3f;
		color += g_SSAO.Sample(g_Sampler, input.UV).r * ambient;
	}
	
	return float4(color.rgb,  parameters.Diffuse.a);
}
 	