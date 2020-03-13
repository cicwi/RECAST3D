R"(
#version 150

in vec3 in_position;
out vec2 tex_coord;
out vec3 volume_coord;

uniform mat4 world_to_screen_matrix;
uniform mat4 orientation_matrix;

void main() {
    tex_coord = vec2(in_position.x, in_position.y);
    vec3 world_position = (orientation_matrix * vec4(in_position, 1.0f)).xyz;
    volume_coord = 0.5f * (world_position + vec3(1.0f));
    gl_Position = world_to_screen_matrix * vec4(world_position, 1.0f);
}
)"
