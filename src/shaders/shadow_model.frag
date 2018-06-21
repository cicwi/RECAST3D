R"(
#version 330

in vec3 fragment_position;

out vec4 fragColor;

struct ads_material {
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    float opacity;
    float shininess;
};

uniform ads_material material;

void main() {
    fragColor = vec4((vec3(1.0f) - material.diffuse_color) * vec3(material.opacity * 0.1f), 1.0f);
}
)"
