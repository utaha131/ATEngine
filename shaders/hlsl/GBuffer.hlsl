struct VertexIn {
	float3 Position : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float2 UV : TEXCOORD;
};

struct PixelIn {
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
	float3x3 TBN : TBN_MATRIX;
};

struct Output {
	float4 g_BaseColor;
	float4 g_Normal;
	float4 g_Surface;
};

[[vk::binding(0, 0)]] cbuffer cbPerMaterial : register(b0, space0) {
	float3 Base_Color_Factor;
	float3 Roughness_Metalness_Factor;
};
[[vk::binding(1, 0)]] Texture2D Base_Color_Map : register(t0);
[[vk::binding(2, 0)]] Texture2D Normal_Map : register(t1);
[[vk::binding(3, 0)]] Texture2D Roughness_Metalness_Map : register(t2);
[[vk::binding(4, 0)]] SamplerState g_MaterialSampler : register(s0);

[[vk::binding(0, 1)]] cbuffer ObjectMatrices : register(b0, space1) {
	float4x4 Model_View_Projection_Matrix;
	float4x4 Normal_Matrix;
};

static const float3 F0_DIELECTRICS = float3(0.04f, 0.04f, 0.04f);

PixelIn VS(VertexIn input) {
	PixelIn output;
	output.Position = mul(Model_View_Projection_Matrix, float4(input.Position.xyz, 1.0f));
	output.UV = input.UV;
	float3 T = normalize(mul((float3x3)Normal_Matrix, normalize(input.Tangent)));
	float3 B = normalize(mul((float3x3)Normal_Matrix, normalize(input.Bitangent)));
	float3 N = normalize(mul((float3x3)Normal_Matrix, normalize(input.Normal)));
	output.TBN = transpose(float3x3(T, B, N));
	return output;
}


Output PS(PixelIn input) : SV_Target {
	Output output;
	float3 normal = Normal_Map.Sample(g_MaterialSampler, input.UV).rgb;
	normal = normalize(normal * 2.0f - 1.0f);
	normal = normalize(mul(input.TBN, normal)) * 0.5f + 0.5f;
	output.g_Normal = float4(normal.xyz, 1.0f);
	float4 base_color = Base_Color_Map.Sample(g_MaterialSampler, input.UV).rgba;
	output.g_BaseColor = float4(base_color.rgb * Base_Color_Factor, base_color.a);
	output.g_Surface = float4(Roughness_Metalness_Map.Sample(g_MaterialSampler, input.UV).rgb * float3(1.0f, Roughness_Metalness_Factor.g, Roughness_Metalness_Factor.b), 1.0f);
	return output;
}