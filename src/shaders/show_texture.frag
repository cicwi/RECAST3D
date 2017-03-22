#version 150

in vec2 tex_coord;

uniform sampler2D texture_sampler;

out vec4 fragColor;

void main() {
    float value = texture(texture_sampler, tex_coord).x;
    fragColor = vec4(vec3(value), 1.0f);
}
