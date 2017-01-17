#version 150

in vec3 in_position;
out vec2 tex_coord;

uniform mat4 transform_matrix;

void main() {
    tex_coord = vec2(in_position.x, in_position.y);
    gl_Position = transform_matrix *
                  vec4(in_position.x, in_position.y, in_position.z, 1.0f);
}
