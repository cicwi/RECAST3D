#version 150

in vec2 in_position;
in vec3 in_color;

out vec4 out_color;

uniform sampler2D texture_sampler;
uniform float size;

void main() {
    vec2 offset = vec2(-0.95 + (gl_InstanceID / 20) * 0.1,
                       -0.95 + (gl_InstanceID % 20) * 0.1);

    vec2 position =
        vec2(offset.x + size * in_position.x, offset.y + size * in_position.y);

    gl_Position = vec4(position.x, position.y, 0.0, 1.0);

    out_color = texture(texture_sampler, 0.5 * (offset + vec2(1.0)));
}
