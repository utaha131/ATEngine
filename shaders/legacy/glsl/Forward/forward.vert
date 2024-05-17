#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_tangent;
layout(location = 3) in vec3 vertex_bitangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec4 vs_out_fragment_position;
layout(location = 1) out vec3 vs_out_normal;
layout(location = 2) out vec3 vs_out_tangent;
layout(location = 3) out vec2 vs_out_uv;
layout(location = 4) out mat3 TBN;

layout (push_constant) uniform constants {
    mat4 mvp_mat;
    mat4 model_view_mat;
    mat4 model_mat;
} mvp;

struct Light {
	vec4 Position_Or_Direction;
	int Type;
	float Intensity;
	float Radius;
};

void main() {
    gl_Position = mvp.mvp_mat * vec4(vertex_position, 1.0);
    vs_out_fragment_position = (mvp.model_view_mat * vec4(vertex_position, 1.0f));
    vs_out_normal = mat3(mvp.model_mat) * vertex_normal;
    vs_out_tangent = vertex_tangent;
    vs_out_uv = uv;
    vec3 T = mat3(mvp.model_mat) * vertex_tangent;
    vec3 B = mat3(mvp.model_mat) * vertex_bitangent;
    vec3 N = vs_out_normal;
    TBN = mat3(T, B, N);
}