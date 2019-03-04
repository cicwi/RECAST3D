R"(
#version 150

in vec2 tex_coord;
in vec3 volume_coord;

uniform sampler2D texture_sampler;
uniform sampler1D colormap_sampler;
uniform sampler3D volume_data_sampler;

uniform int transparency_mode;
uniform int hovered;
uniform int has_data;
uniform float min_value;
uniform float max_value;
uniform float volume_min_value;
uniform float volume_max_value;

out vec4 fragColor;

void main() {
    float value = 0.0f;

    if (has_data != 1) {
       value = (texture(volume_data_sampler, volume_coord).x - volume_min_value)
                 / (volume_max_value - volume_min_value);
    } else {
        value = texture(texture_sampler, tex_coord).x;
        value = (value - min_value) / (max_value - min_value);
    }

    fragColor = vec4(texture(colormap_sampler, value).xyz, 1.0f);

    if (has_data != 1) {
        fragColor.a = 0.75f;
    }

    if (transparency_mode != 0) {
        if (value < 0.1f) { discard; }
    }

    if (hovered == 1) {
        fragColor += vec4(0.3f);
    }
}
)"
