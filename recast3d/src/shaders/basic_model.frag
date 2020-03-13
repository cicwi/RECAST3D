R"(
#version 330

in vec3 varying_normal;
in vec3 fragment_position;

out vec4 fragColor;

uniform vec3 camera_position;

struct ads_material {
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    float opacity;
    float shininess;
};

uniform ads_material material;

void main() {
    vec3 normal = normalize(varying_normal);
    vec3 light_direction = normalize(vec3(0.4f, 0.4f, 0.8f));

    vec3 view_direction = normalize(camera_position - fragment_position);
    vec3 reflect_direction = reflect(-light_direction, normal);

    vec3 ambient = material.ambient_color * material.diffuse_color;
    vec3 diffuse = max(dot(light_direction, normal), 0.0f) * material.diffuse_color;
    vec3 specular = pow(max(dot(view_direction, reflect_direction), 0.0f), material.shininess) * material.specular_color;

    fragColor = vec4(ambient + diffuse + specular, material.opacity);
}
)"
