#version 330

in vec2 tex_coord;

uniform sampler2D texture_sampler;

out vec4 color;

void main() {
    color = vec4(0.2f) + vec4(vec3(texture(texture_sampler, tex_coord).x), 1.0f);
}
