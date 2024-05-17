struct VertexIn {
	float3 Position : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float2 UV : TEXCOORD;
};

struct PixelIn {
	float4 Position : SV_POSITION;
};

[[vk::binding(0, 0)]] cbuffer ObjectMatrices : register(b0) {
	float4x4 Model_View_Projection_Matrix;
};

PixelIn VS(VertexIn input) {
	PixelIn output;
	float4 position = float4(input.Position, 1.0f);
	output.Position = mul(Model_View_Projection_Matrix, position);
	return output;
}

void PS(PixelIn input) {
	
}