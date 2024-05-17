#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;

layout(location = 0) out vec2 vs_out_uv;

void main() {
	gl_Position = vec4(vertex_position, 1.0f);
	gl_Position.y *= -1.0f;
	vs_out_uv = vertex_uv;
}