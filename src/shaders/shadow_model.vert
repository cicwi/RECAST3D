R"(
#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

uniform mat4 world_matrix;
uniform mat4 model_matrix;
uniform mat4 mesh_rotate;
uniform mat4 mesh_translate;

out vec3 fragment_position;

void main() {
    gl_Position = world_matrix * model_matrix * mesh_translate * mesh_rotate * vec4(in_position.x, in_position.y, in_position.z, 1.0f);
    fragment_position = in_position;
}
)"
