#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

uniform mat4 world_matrix;
uniform mat4 model_matrix;
uniform mat4 mesh_matrix;

out vec3 normal;
out vec3 fragment_position;

void main() {
    gl_Position = world_matrix * model_matrix * mesh_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0f);
    normal = (model_matrix * vec4(in_normal, 1.0f)).xyz;
    fragment_position = in_position;
}
