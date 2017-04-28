#version 330

in vec3 normal;
in vec3 fragment_position;

out vec4 fragColor;

uniform vec3 camera_position;

struct ads_material {
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    float opacity;
    int shininess;
};

uniform ads_material material;

void main() {
    vec3 nnormal = normalize(normal);
    vec3 light_direction = normalize(vec3(-0.6f, -0.6f, -0.8f));

    vec3 view_direction = normalize(camera_position - fragment_position);
    vec3 reflect_direction = reflect(-light_direction, nnormal);

    vec3 ambient = material.ambient_color * material.diffuse_color;
    vec3 diffuse = max(dot(light_direction, nnormal), 0.0f) * material.diffuse_color;
    vec3 specular = pow(max(dot(view_direction, reflect_direction), 0.0f), material.shininess) * material.specular_color;

    fragColor = vec4(ambient + diffuse + specular, 1.0f);
}
