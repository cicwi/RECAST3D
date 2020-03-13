R"(
#version 330

in vec3 in_position;
out vec2 tex_coord;

uniform mat4 world_to_screen_matrix;
uniform mat4 orientation_matrix;

void main() {
    tex_coord = vec2(in_position.x, in_position.y);
    vec3 world_position = (orientation_matrix * vec4(tex_coord, 1.0f, 1.0f)).xyz;
    gl_Position = world_to_screen_matrix * vec4(world_position, 1.0f);
}
)"
