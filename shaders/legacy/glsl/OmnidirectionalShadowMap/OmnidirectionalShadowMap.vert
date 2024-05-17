#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_tangent;
layout(location = 3) in vec3 vertex_bitangent;
layout(location = 4) in vec2 vertex_uv;

layout(location = 0) out vec4 vs_out_fragment_position;

layout (set = 0, binding = 0 ) uniform UniformBufferObject {
    mat4 Model_View_Projection_Matrix;
    mat4 Model_Matrix;
    vec4 Light_Position;
    float Max_Distance;
} constants;

void main() {
    vec4 position4 = vec4(vertex_position, 1.0f);
    gl_Position = constants.Model_View_Projection_Matrix * position4;
    vs_out_fragment_position = constants.Model_Matrix * position4;
}