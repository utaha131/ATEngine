struct Light {
	float4 PositionOrDirection;
	float3 Color;
	float4x4 Matrices[4];
	int Type;
	float Intensity;
	float Radius;
};
