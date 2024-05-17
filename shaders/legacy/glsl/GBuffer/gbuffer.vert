#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_tangent;
layout(location = 3) in vec3 vertex_bitangent;
layout(location = 4) in vec2 vertex_uv;


layout(location = 0) out vec2 vs_out_uv;
layout(location = 1) out mat3 TBN;

layout(set = 1, binding = 0) uniform UniformBufferObject {
	mat4 Model_View_Projection_Matrix;
    mat4 Normal_Matrix;
} constants;

void main() {
    gl_Position = constants.Model_View_Projection_Matrix * vec4(vertex_position.xyz, 1.0f);
    vs_out_uv = vertex_uv;
    vec3 T = normalize(mat3(constants.Normal_Matrix) * vertex_tangent);
    vec3 B = normalize(mat3(constants.Normal_Matrix) * vertex_bitangent);
    vec3 N = normalize(mat3(constants.Normal_Matrix) * vertex_normal);
    TBN = mat3(T, B, N);
}