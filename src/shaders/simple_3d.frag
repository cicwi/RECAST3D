#version 150

in vec2 tex_coord;

uniform sampler2D texture_sampler;
uniform sampler1D colormap_sampler;

uniform int hovered;
uniform int has_data;

out vec4 fragColor;

void main() {
    float value = texture(texture_sampler, tex_coord).x;
    fragColor = vec4(texture(colormap_sampler, value).xyz, 1.0f);
    if (hovered == 1) {
        fragColor += vec4(0.3f);
    }

    if (has_data != 1) {
        fragColor.a = 0.5f;
    }
}
