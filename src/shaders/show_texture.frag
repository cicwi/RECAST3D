R"(
#version 150

in vec2 tex_coord;

uniform sampler2D texture_sampler;
uniform sampler1D colormap_sampler;

out vec4 fragColor;

void main() {
    float value = texture(texture_sampler, tex_coord).x;
    fragColor = vec4(texture(colormap_sampler, value).xyz, 1.0f);
}
)"
