#version 150

in vec2 in_position;
in vec3 in_color;

out vec3 out_color;

void main() {
    vec2 offset = vec2(-0.9 + (gl_InstanceID / 10) * 0.2,
                       -0.9 + (gl_InstanceID % 10) * 0.2);
    float size = 0.09;
    gl_Position = vec4(offset.x + size * in_position.x,
                       offset.y + size * in_position.y, 0.0, 1.0);

    out_color = in_color;
}
