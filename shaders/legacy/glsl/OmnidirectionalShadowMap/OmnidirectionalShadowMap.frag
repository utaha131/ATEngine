#version 450

layout(location = 0) in vec4 vs_out_fragment_position;

/*layout (push_constant) uniform MatrixConstants {
    mat4 Model_View_Projection_Matrix;
    mat4 Model_Matrix;
    vec4 Light_Position;
} constants;*/

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 Model_View_Projection_Matrix;
    mat4 Model_Matrix;
    vec4 Light_Position;
    float Max_Distance;
} constants;

void main() {
    gl_FragDepth = distance(constants.Light_Position.xyz, vs_out_fragment_position.xyz / vs_out_fragment_position.w) / constants.Max_Distance;
}