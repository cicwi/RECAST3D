#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

uniform mat4 transform_matrix;

out vec3 normal;

void main() {
    gl_Position = transform_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0f);
    normal = in_normal;
}
