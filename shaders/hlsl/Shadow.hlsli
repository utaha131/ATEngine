inline float CascadedShadowMapFactor(float3 Fragment_Position, float3 Fragment_World_Position, float4x4 Light_Matrix[4], Texture2DArray Shadow_Map, SamplerState g_Sampler) {
	float shadow = 0.0f;
	float current_depth = abs(Fragment_Position.z);
	uint map_index = 0;
	if (current_depth < 20.0f) {
		map_index = 0;
	} else if (current_depth < 100.0f) {
		map_index = 1;
	} else if (current_depth < 300.0f) {
		map_index = 2;
	} else if (current_depth < 600.0f) {
		map_index = 3;
	}


	float4 light_space_position = mul(Light_Matrix[map_index], float4(Fragment_World_Position.xyz, 1.0f));
	float3 uv = light_space_position.xyz / light_space_position.w * 0.5f  + 0.5f;
	uv.y = 1.0f - uv.y;
	float texel_size = 1.0f / 4096.0f;

	float test = light_space_position.z / light_space_position.w;
	
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			float depth = Shadow_Map.Sample(g_Sampler, float3(uv.x + (j * texel_size), uv.y + (i * texel_size), map_index)).r;
			
			if (test + 0.002f < depth) {
				shadow += 0.0f;
			} else {
				shadow += 1.0f;
			}
		}
	}
	shadow /= 9.0f;
	
	return shadow;
}

inline float OmnidirectionalShadowMapFactor(float3 Fragment_World_Position, float3 Light_View_Position, float4x4 Inverse_View_Matrix, TextureCube g_ShadowMap, SamplerState g_Sampler, float Radius) {
	float shadow = 0.0f;
	float4 light_world_pos = mul(Inverse_View_Matrix, float4(Light_View_Position.xyz, 1.0f));
	light_world_pos.xyz /= light_world_pos.w;
	float3 world_dir = Fragment_World_Position - light_world_pos.xyz;
	float current_depth = distance(light_world_pos, Fragment_World_Position) / Radius; 
	float texel_size = 1.0f / 1024.0f;

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			for (int k = -1; k <= 1; ++k) {
				float depth = g_ShadowMap.Sample(g_Sampler, normalize(world_dir + float3(j * texel_size, i * texel_size, k * texel_size))).r;
				if (current_depth > depth) {
					shadow += 0.0f;
				} else {
					shadow += 1.0f;
				}
			}
		}
	}
	shadow /= 27.0f;

	return shadow;
}