struct VertexIn {
	float3 Position : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float2 UV : TEXCOORD;
};

struct PixelIn {
	float4 Position : SV_POSITION;
	float4 Fragment_Position: FRAGMENT_POSITION;
};


[[vk::binding(0, 0)]] cbuffer ObjectMatrices : register(b0) {
	float4x4 Light_Projection_Matrix;
	float4x4 Model_Matrix;
	float4 Light_Position;
	float Max_Distance;
};



PixelIn VS(VertexIn input) {
	PixelIn output;
	float4 position = float4(input.Position, 1.0f);
	output.Fragment_Position = mul(Model_Matrix, position);
	output.Position = mul(Light_Projection_Matrix, position);
	return output;
}

float PS(PixelIn input) : SV_Depth {
	return distance(Light_Position.xyz, input.Fragment_Position.xyz / input.Fragment_Position.w) / Max_Distance;
}