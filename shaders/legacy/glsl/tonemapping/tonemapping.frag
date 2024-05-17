#version 450

layout(location = 0) in vec2 vs_out_uv;

layout(set = 0, binding = 0) uniform texture2D g_Frame;
layout(set = 0, binding = 1) uniform sampler g_Sampler;

layout(location = 0) out vec4 output_color;

void main() {
	const float gamma = 2.2f;
	vec4 color = texture(sampler2D(g_Frame, g_Sampler), vs_out_uv).rgba;
	vec3 ldr = color.rgb / (color.rgb + vec3(1.0f));
	ldr = pow(ldr, vec3(1.0f / gamma));

	output_color = vec4(ldr, 1.0f);
}