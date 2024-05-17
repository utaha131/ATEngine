struct Light {
	float4 Position_Or_Direction;
	float4x4 Matrices[4];
	int Type;
	float Intensity;
	float Radius;
};
