#version 150

in vec2 tex_coord;
in vec3 volume_coord;

uniform sampler2D texture_sampler;
uniform sampler1D colormap_sampler;
uniform sampler3D volume_data_sampler;

uniform int hovered;
uniform int has_data;
uniform float min_value;
uniform float max_value;

out vec4 fragColor;

void main() {
    float value = texture(texture_sampler, tex_coord).x;

    if (has_data != 1) {
        value = texture(volume_data_sampler, volume_coord).x;
    }

    if (max_value != min_value) {
        value = (value - min_value) / (max_value - min_value);
    }

    fragColor = vec4(texture(colormap_sampler, value).xyz, 1.0f);

    if (has_data != 1) {
        fragColor.a = 0.75f;
    }

    if (hovered == 1) {
        fragColor += vec4(0.3f);
    }
}
