#version 450

layout (push_constant) uniform constants {
    mat4 mvp_mat;
    mat4 model_view_mat;
    mat4 model_mat;
} mvp;

layout(location = 0) in vec4 vs_out_fragment_position;
layout(location = 1) in vec3 vs_out_normal;
layout(location = 2) in vec3 vs_out_tangent;
layout(location = 3) in vec2 vs_out_uv;
layout(location = 4) in mat3 TBN;

layout(set = 0, binding = 0) uniform texture2D diffuseMap;
layout(set = 0, binding = 1) uniform texture2D normalMap;
layout(set = 0, binding = 2) uniform texture2D roughnessMap;
layout(set = 0, binding = 3) uniform sampler diffuseSampler;

struct Light {
	vec4 Position_Or_Direction;
	int Type;
	float Intensity;
	float Radius;
};

layout(set = 1, binding = 0) uniform UniformBufferObject {
	Light lights[4];
} ubo;


layout(location = 0) out vec4 out_color;

vec3 BGR(vec3 color) {
    return vec3(color.b, color.g, color.r);
}


const float PI = 3.14159265359f;
const float INVERSE_PI = 1.0f / 3.14159265359f;
const float F0_Dielectrics = 0.04f;

struct BSDF_Parameters {
	vec3 L;
	vec3 N;
	vec3 V;
	vec3 H;
	float NdotL;
	float NdotV;
	float LdotH;
	float NdotH;
	float VdotH;
	vec3 Base_Color;
	float Metalness;
	float Roughness;
	vec3 Specular_Color;
};


vec3 Lambertian_Diffuse(vec3 base_color, float metalness) {
	float kD = 1.0f - metalness;
	return (base_color * vec3(kD, kD, kD)) * INVERSE_PI;
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


vec3 BRDF(BSDF_Parameters parameters) {
	vec3 Diffuse = Lambertian_Diffuse(parameters.Base_Color, parameters.Metalness);
	float D = Distribution_GGX(parameters.Roughness * parameters.Roughness, parameters.NdotH);
	vec3 F = Fresnel_SchlickApprox(parameters.Specular_Color, 1.0f, parameters.VdotH);
	float G = Geometric(parameters.Roughness, parameters.NdotV, parameters.NdotL);
	vec3 Specular = (D * F * G) / (4.0f * parameters.NdotL * parameters.NdotV + 0.00001f);
	return Diffuse + Specular;
}

void main() {
    vec3 base_color = BGR(texture(sampler2D(diffuseMap, diffuseSampler), vs_out_uv).rgb);
    vec3 normal = BGR(texture(sampler2D(normalMap, diffuseSampler), vs_out_uv).rgb);
    normal = normal * 2.0f - 1.0f;
    normal = normalize(TBN * normal);
    vec3 roughness_metal = BGR(texture(sampler2D(roughnessMap, diffuseSampler), vs_out_uv).rgb);
	float roughness = roughness_metal.g;
	float metalness = roughness_metal.b;
	vec4 diffuse = texture(sampler2D(diffuseMap, diffuseSampler), vs_out_uv).rgba;

	BSDF_Parameters parameters;
	parameters.V = normalize(-vs_out_fragment_position.rgb);
	parameters.N = normal;
	parameters.NdotV = clamp(dot(parameters.N, parameters.V), 0.00001f, 1.0f);
	parameters.Base_Color = base_color.rgb;
	parameters.Metalness = min(max(metalness, 0.0f), 1.0f);
	parameters.Roughness = min(max(roughness, 0.0f), 1.0f);
	parameters.Specular_Color = mix(vec3(F0_Dielectrics, F0_Dielectrics, F0_Dielectrics), parameters.Base_Color, vec3(metalness, metalness, metalness));

	vec3 color = vec3(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < 4; ++i) {
		float attenuation = 1.0f;
		if (ubo.lights[i].Type == 0) { //Directional Light
			parameters.L = normalize(ubo.lights[i].Position_Or_Direction.rgb);
		} else if (ubo.lights[i].Type == 1) {
			float d = distance(ubo.lights[i].Position_Or_Direction.rgb, vs_out_fragment_position.rgb);
			attenuation = clamp((1.0f - pow(d / ubo.lights[i].Radius, 4.0f)), 0.0f, 1.0f);
			attenuation =  attenuation * attenuation / (d * d + 1.0f);
			parameters.L = normalize(ubo.lights[i].Position_Or_Direction.rgb - vs_out_fragment_position.rgb);
		}
		parameters.H = normalize(parameters.L + parameters.V);
		parameters.NdotL = clamp(dot(parameters.N, parameters.L), 0.00001f, 1.0f);
		parameters.LdotH = clamp(dot(parameters.L, parameters.H), 0.00001f, 1.0f);
		parameters.NdotH = clamp(dot(parameters.N, parameters.H), 0.00001f, 1.0f);
		parameters.VdotH = clamp(dot(parameters.V, parameters.H), 0.00001f, 1.0f);
		color += BRDF(parameters) * ubo.lights[i].Intensity * attenuation * parameters.NdotL;
	}

    out_color = vec4(clamp(/*color*/normal.rgb, 0.0f, 1.0f).rgb,  diffuse.a);
}