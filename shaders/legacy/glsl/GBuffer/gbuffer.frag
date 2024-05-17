#version 450

layout(location = 0) in vec2 vs_out_uv;
layout(location = 1) in mat3 TBN;

layout(location = 0) out vec4 g_BaseColor;
layout(location = 1) out vec4 g_Normal;
layout(location = 2) out vec4 g_Surface;

layout(set = 0, binding = 0) uniform texture2D Base_Color_Map;
layout(set = 0, binding = 1) uniform texture2D Normal_Map;
layout(set = 0, binding = 2) uniform texture2D Roughness_Metalness_Map;
layout(set = 0, binding = 3) uniform sampler gSampler;

layout(set = 1, binding = 0) uniform UniformBufferObject {
	mat4 Model_View_Projection_Matrix;
    mat4 Normal_Matrix;
} constants;

void main() {
    vec3 normal = texture(sampler2D(Normal_Map, gSampler), vs_out_uv).rgb;
    normal = normalize(normal * 2.0f - 1.0f);
    vec3 roughness_metal = texture(sampler2D(Roughness_Metalness_Map, gSampler), vs_out_uv).rgb;
    g_BaseColor = texture(sampler2D(Base_Color_Map, gSampler), vs_out_uv);
    g_Normal = vec4(normalize(TBN * normal.rgb) * 0.5f + 0.5f, 1.0f);
    g_Surface = vec4(0.0f, roughness_metal.g, roughness_metal.b, 1.0f);
}