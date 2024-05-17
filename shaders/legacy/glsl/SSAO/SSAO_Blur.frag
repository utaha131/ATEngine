#version 450

layout(location = 0) in vec2 vs_out_uv;

layout(set = 0, binding = 0) uniform texture2D g_SSAO;
layout(set = 0, binding = 1) uniform sampler g_Sampler;

layout(location = 0) out float output_color;

void main() {
	vec2 ssao_texel_size = 1.0f / vec2(1280.0f, 720.0f);
	float result = 0.0f;
	for (int i = -2; i < 2; ++i) {
		for (int j = -2; j < 2; ++j) {
			vec2 coordinate = vs_out_uv + (vec2(i, j) * ssao_texel_size);
			result += texture(sampler2D(g_SSAO, g_Sampler), coordinate).r;
		}
	}
	output_color = result / (4.0f * 4.0f);
}