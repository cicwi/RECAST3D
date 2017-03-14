#version 150

in vec2 tex_coord;
in vec3 volume_coord;

uniform sampler2D texture_sampler;
uniform sampler1D colormap_sampler;
uniform sampler3D volume_data_sampler;

uniform int hovered;
uniform int has_data;

out vec4 fragColor;

void main() {
    float value = texture(texture_sampler, tex_coord).x;

    if (has_data != 1) {
        value = texture(volume_data_sampler, volume_coord).x;
    }

    fragColor = vec4(texture(colormap_sampler, value).xyz, 1.0f);

    if (has_data != 1) {
        fragColor.a = 0.3f;
    }

    if (hovered == 1) {
        fragColor += vec4(0.3f);
    }
}
