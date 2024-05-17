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
};

[[vk::binding(0, 0)]] cbuffer ObjectMatrices : register(b0, space0) {
	float4x4 Model_View_Projection_Matrix;
	float4x4 Normal_Matrix;
};

[[vk::binding(1, 0)]] Texture2D Base_Color_Map : register(t0);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0);

PixelIn VS(VertexIn input) {
	PixelIn output;
	output.Position = mul(Model_View_Projection_Matrix, float4(input.Position.xyz, 1.0f));
	output.UV = input.UV;
	return output;
}

void PS(PixelIn input) {
	if (Base_Color_Map.Sample(g_Sampler, input.UV).a <= 0.05f) {
        discard;
    }
}