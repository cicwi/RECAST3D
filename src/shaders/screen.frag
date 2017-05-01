#version 330

in vec2 tex_coord;

uniform sampler2D texture_sampler;

out vec4 color;

void main() {
    color = vec4(0.2f) + texture(texture_sampler, tex_coord);
}
