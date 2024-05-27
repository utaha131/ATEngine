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
	float4x4 ModelViewProjectionMatrix;
	float4x4 ModelMatrix;
	float4 LightPosition;
	float MaxDistance;
};



PixelIn VS(VertexIn input) {
	PixelIn output;
	float4 position = float4(input.Position, 1.0f);
	output.Fragment_Position = mul(ModelMatrix, position);
	output.Position = mul(ModelViewProjectionMatrix, position);
	return output;
}

float PS(PixelIn input) : SV_Depth {
	return distance(LightPosition.xyz, input.Fragment_Position.xyz / input.Fragment_Position.w) / MaxDistance;
}