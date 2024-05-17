#version 450

layout(location = 0) in vec2 vs_out_uv;

layout(location = 0) out vec4 output_color;

#define PI 3.14159265359f
#define INVERSE_PI 1.0f / 3.14159265359f

struct BRDF_Parameters {
	vec3 L;
	vec3 N;
	vec3 V;
	vec3 H;
	float NdotL;
	float NdotV;
	float LdotH;
	float NdotH;
	float VdotH;
	vec4 Diffuse;
	float Roughness; 
	vec3 Specular_Color;
};

struct Light {
	vec4 Position_Or_Direction;
	int Type;
	float Intensity;
	float Radius;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
	Light lights[4];
	mat4 Light_Matrix[4];
	mat4 Inverse_Projection_Matrix;
	mat4 Inverse_View_Matrix;
} constants;

layout(set = 0, binding = 1) uniform texture2D g_BaseColor;
layout(set = 0, binding = 2) uniform texture2D g_Normal;
layout(set = 0, binding = 3) uniform texture2D g_Surface;
layout(set = 0, binding = 4) uniform texture2D g_Depth;
layout(set = 0, binding = 5) uniform texture2D g_SSAO;
layout(set = 0, binding = 6) uniform textureCube g_PointShadowMap;
layout(set = 0, binding = 7) uniform texture2DArray g_DirectionalShadowMap;
layout(set = 0, binding = 8) uniform sampler g_Sampler;

vec3 Lambertian_Diffuse(vec3 diffuse) {
	return diffuse * INVERSE_PI;
}

vec3 Fresnel_SchlickApprox(vec3 F0, float F90, float NdotV) {
	return F0 + (F90 - F0) *  pow(2.0f, (-5.55473f * NdotV - 6.98316f) * NdotV);
}

float Distribution_GGX(float alpha, float NdotH) {
	float alpha2 = max(alpha * alpha, 0.00001f);
	float term  = ( ((alpha2 - 1.0f) * NdotH * NdotH) + 1.0f );
	return alpha2 / (PI * term * term);
}

float Geometric(float roughness, float NdotV, float NdotL) {
	float k = max(roughness * roughness, 0.00001f) * 0.5f;
	float G1 = (NdotV * (1.0f - k) + k);
	float G2 = (NdotL * (1.0f - k) + k);
	return (NdotV * NdotL) / (G1 * G2);
}

vec3 BRDF(BRDF_Parameters parameters) {
	vec3 Diffuse = Lambertian_Diffuse(parameters.Diffuse.rgb);
	float D = Distribution_GGX(parameters.Roughness * parameters.Roughness, parameters.NdotH);
	vec3 F = Fresnel_SchlickApprox(parameters.Specular_Color, 1.0f, parameters.VdotH);
	float G = Geometric(parameters.Roughness, parameters.NdotV, parameters.NdotL);
	vec3 Specular = (D * F * G) / (4.0f * parameters.NdotL * parameters.NdotV + 0.00001f);
	return Diffuse + Specular; 
}

float CascadedShadowMapFactor(vec3 Fragment_Position, vec3 Fragment_World_Position, texture2DArray Shadow_Map, sampler Sampler) {
	float shadow = 1.0f;
	float current_depth = abs(Fragment_Position.z);
	int map_index = 0;
	if (current_depth < 20.0f) {
		map_index = 0;
	} else if (current_depth < 100.0f) {
		map_index = 1;
	} else if (current_depth < 250.0f) {
		map_index = 2;
	} else if (current_depth < 500.0f) {
		map_index = 3;
	}	

	vec4 light_space_position = (constants.Light_Matrix[map_index] * vec4(Fragment_World_Position, 1.0f));
	vec3 uv = light_space_position.xyz / light_space_position.w * 0.5f  + 0.5f;
	uv.y = 1.0f - uv.y;
	float depth = 0.0f;

	float texel_size = 1.0f / 2048;
	shadow = 0.0f;
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			depth = texture(sampler2DArray(Shadow_Map, Sampler), vec3(uv.x + texel_size * j, uv.y + texel_size * i, map_index)).r;
			float test = clamp(light_space_position.z / light_space_position.w, 0.0f, 1.0f);
			if (test + 0.0002f < depth) {
				shadow += 0.0f;
			} else {
				shadow += 1.0f;
			}
		}
	}
	shadow /= 9.0f;

	return shadow;
}

float OmnidirectionalShadowMapFactor(vec3 Light_Position, vec3 Fragment_World_Position, textureCube Shadow_Map, sampler Sampler) {
	float shadow = 1.0f;
	vec4 light_world_pos = (constants.Inverse_View_Matrix * vec4(Light_Position.xyz, 1.0f));
	light_world_pos.xyz /= light_world_pos.w;
	vec3 world_dir = Fragment_World_Position - light_world_pos.xyz;
	float current_depth = distance(light_world_pos.xyz, Fragment_World_Position) / 5.0f;
	
	float depth = 0.0f;
	float texel_size = 1.0f / 1024;
	shadow = 0.0f;
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			for (int k = -1; k <= 1; ++k) {
				depth =  texture(samplerCube(Shadow_Map, Sampler), normalize(world_dir + (j, i, k) * texel_size)).r;
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

#define F0_DIELECTRICS vec3(0.04f, 0.04f, 0.04f)

void main() {
	BRDF_Parameters parameters;
	float depth = texture(sampler2D(g_Depth, g_Sampler), vs_out_uv).r;
	vec2 coords = vec2(vs_out_uv.x, vs_out_uv.y) * 2.0f - 1.0f;
	vec4 Fragment_Position = (constants.Inverse_Projection_Matrix * vec4(coords.xy, depth, 1.0f)).xyzw;
	Fragment_Position.xyz /= Fragment_Position.w;
	vec4 Surface = texture(sampler2D(g_Surface, g_Sampler), vs_out_uv).rgba;
	parameters.V = normalize(-Fragment_Position.xyz);
	parameters.N = normalize(texture(sampler2D(g_Normal, g_Sampler), vs_out_uv).xyz * 2.0f - 1.0f);
	parameters.NdotV = clamp(dot(parameters.N, parameters.V), 0.0f, 1.0f);
	vec4 base_color = texture(sampler2D(g_BaseColor, g_Sampler), vs_out_uv).rgba;
	parameters.Diffuse = vec4(base_color.rgb * (1.0f - Surface.b), base_color.a);
	parameters.Roughness = Surface.g;
	parameters.Specular_Color = vec3(mix(F0_DIELECTRICS, base_color.rgb, Surface.bbb).rgb).rgb;
	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	vec4 Fragment_World_Position = (constants.Inverse_View_Matrix * vec4(Fragment_Position.xyz, 1.0f)).xyzw;
	Fragment_World_Position.xyz /=  Fragment_World_Position.w;
	for(int i = 0; i < 4; ++i) {
		float shadow = 1.0f;
		float attenuation = 1.0f;
		if (constants.lights[i].Type == 0) { //Directional Light
			parameters.L = normalize(constants.lights[i].Position_Or_Direction.rgb);
			shadow = CascadedShadowMapFactor(Fragment_Position.xyz, Fragment_World_Position.xyz, g_DirectionalShadowMap, g_Sampler);
		} else if (constants.lights[i].Type == 1) {
			float d = distance(constants.lights[i].Position_Or_Direction.rgb, Fragment_Position.xyz);
			attenuation = clamp(1.0f - pow(d / constants.lights[i].Radius, 4.0f), 0.0f, 1.0f);
			attenuation =  attenuation * attenuation / (d * d + 1.0f);
			parameters.L = normalize(constants.lights[i].Position_Or_Direction.xyz - Fragment_Position.xyz);
			shadow = OmnidirectionalShadowMapFactor(constants.lights[i].Position_Or_Direction.xyz, Fragment_World_Position.xyz, g_PointShadowMap, g_Sampler);
			//output_color = vec4(shadow, shadow, shadow, 1.0f); return;
		}
		parameters.H = normalize(parameters.L + parameters.V);
		parameters.NdotL = clamp(dot(parameters.N, parameters.L), 0.0f, 1.0f);
		parameters.LdotH = clamp(dot(parameters.L, parameters.H), 0.0f, 1.0f);
		parameters.NdotH = clamp(dot(parameters.N, parameters.H), 0.0f, 1.0f);
		parameters.VdotH = clamp(dot(parameters.V, parameters.H), 0.0f, 1.0f);
		color += BRDF(parameters) * constants.lights[i].Intensity * attenuation * parameters.NdotL * shadow;
	}
	color += texture(sampler2D(g_SSAO, g_Sampler), vs_out_uv).r * parameters.Diffuse.rgb * 0.1f;
	output_color = vec4(color.rgb,  parameters.Diffuse.a);
}