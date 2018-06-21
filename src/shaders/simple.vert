R"(
#version 150

in vec2 in_position;
out vec2 tex_coord;

uniform float size;

uniform float aspect_ratio;
uniform float inv_aspect_ratio;

uniform mat4 transform_matrix;

void main() {
    vec2 position = vec2(in_position.x, in_position.y);
    gl_Position =
        transform_matrix * vec4(aspect_ratio * position.x,
                                inv_aspect_ratio * position.y, 0.0f, 1.0f);
    tex_coord = 0.5f * (position + vec2(1.0f));
}
)"
