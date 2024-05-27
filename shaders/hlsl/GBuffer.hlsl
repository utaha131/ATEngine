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
	float4 g_BaseColorTexture;
	float4 g_NormalTexture;
	float4 g_SurfaceTexture;
};

[[vk::binding(0, 0)]] cbuffer cbPerMaterial : register(b0, space0) {
	float3 BaseColorFactor;
	float3 RoughnessMetalnessFactor;
};
[[vk::binding(1, 0)]] Texture2D g_BaseColorMap : register(t0);
[[vk::binding(2, 0)]] Texture2D g_NormalMap : register(t1);
[[vk::binding(3, 0)]] Texture2D g_RoughnessMetalnessMap : register(t2);
[[vk::binding(4, 0)]] SamplerState g_MaterialSampler : register(s0);

[[vk::binding(0, 1)]] cbuffer ObjectMatrices : register(b0, space1) {
	float4x4 ModelViewProjectionMatrix;
	float4x4 NormalMatrix;
};

static const float3 F0_DIELECTRICS = float3(0.04f, 0.04f, 0.04f);

PixelIn VS(VertexIn input) {
	PixelIn output;
	output.Position = mul(ModelViewProjectionMatrix, float4(input.Position.xyz, 1.0f));
	output.UV = input.UV;
	float3 T = normalize(mul((float3x3)NormalMatrix, normalize(input.Tangent)));
	float3 B = normalize(mul((float3x3)NormalMatrix, normalize(input.Bitangent)));
	float3 N = normalize(mul((float3x3)NormalMatrix, normalize(input.Normal)));
	output.TBN = transpose(float3x3(T, B, N));
	return output;
}


Output PS(PixelIn input) : SV_Target {
	Output output;
	float3 normal = g_NormalMap.Sample(g_MaterialSampler, input.UV).rgb;
	normal = normalize(normal * 2.0f - 1.0f);
	normal = normalize(mul(input.TBN, normal)) * 0.5f + 0.5f;
	output.g_NormalTexture = float4(normal.xyz, 1.0f);
	float4 base_color = g_BaseColorMap.Sample(g_MaterialSampler, input.UV).rgba;
	output.g_BaseColorTexture = float4(base_color.rgb * BaseColorFactor, base_color.a);
	output.g_SurfaceTexture = float4(g_RoughnessMetalnessMap.Sample(g_MaterialSampler, input.UV).rgb * float3(1.0f, RoughnessMetalnessFactor.g, RoughnessMetalnessFactor.b), 1.0f);
	return output;
}