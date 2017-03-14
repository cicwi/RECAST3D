#version 150

in vec3 in_position;
out vec2 tex_coord;
out vec3 volume_coord;

uniform mat4 world_to_screen_matrix;
uniform mat4 orientation_matrix;

void main() {
    tex_coord = vec2(in_position.x, in_position.y);
    volume_coord = (orientation_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0f)).xyz;
    gl_Position = world_to_screen_matrix * vec4(volume_coord, 1.0);
}
