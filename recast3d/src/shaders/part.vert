R"(
#version 150

in vec3 in_position;

uniform mat4 transform_matrix;

void main() {
    gl_Position = transform_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0f);
}
)"
