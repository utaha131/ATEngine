#version 450

layout(location = 0) in vec2 vs_out_uv;

layout(location = 0) out float output_color;

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 Inverse_Projection_Matrix;
	mat4 Projection_Matrix;
	vec4 Kernel[64];
} constants;
layout(set = 0, binding = 1) uniform texture2D g_Depth;
layout(set = 0, binding = 2) uniform texture2D g_Normal;
layout(set = 0, binding = 3) uniform texture2D g_Noise;
layout(set = 0, binding = 4) uniform sampler g_Sampler;

void main() {
	float depth = texture(sampler2D(g_Depth, g_Sampler), vs_out_uv).r;
	vec2 coords = vec2(vs_out_uv.x, vs_out_uv.y) * 2.0f - 1.0f;
	vec4 view_position = (constants.Inverse_Projection_Matrix * vec4(coords.xy, depth, 1.0f));
	view_position.xyz = view_position.xyz / view_position.w;
	vec3 normal = normalize(texture(sampler2D(g_Normal, g_Sampler), vs_out_uv).xyz * 2.0f - 1.0f);
	vec2 scale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);
	vec3 random_vector = normalize(texture(sampler2D(g_Noise, g_Sampler), vs_out_uv * scale).xyz  * 2.0f - 1.0f);
	vec3 tangent = normalize(random_vector - normal * dot(random_vector, normal));
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 TBN = mat3(tangent, bitangent, normal);

	vec3 fragment_position = view_position.xyz;

	float occlusion = 0.0f;
	float radius = 0.5f;
	for (int i = 0; i < 64; ++i) {
		vec3 sample_position = TBN * constants.Kernel[i].xyz;
		sample_position = fragment_position + sample_position * radius;

		vec4 offset = vec4(sample_position, 1.0f);
		offset = (constants.Projection_Matrix * offset);
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5f + 0.5f;
		//offset.y = 1.0f - offset.y;
		float sample_depth = texture(sampler2D(g_Depth, g_Sampler), offset.xy).r;
		
		vec4 sample_depth_position = (constants.Inverse_Projection_Matrix * vec4(0.0f, 0.0f, sample_depth, 1.0f));
		sample_depth = sample_depth_position.z / sample_depth_position.w;

		float bias = 0.03f;

		float range_check = smoothstep(0.0, 1.0, radius / abs(fragment_position.z - sample_depth));
		occlusion += (sample_depth >= sample_position.z + bias ? 1.0 : 0.0) * range_check;  
	}

	occlusion = 1.0f - (occlusion / 64.0f);
	output_color = occlusion;
}