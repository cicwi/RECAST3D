#version 330

in vec3 normal;
in vec3 fragment_position;

out vec4 fragColor;

uniform vec3 camera_position;

void main() {
    vec3 nnormal = normalize(normal);
    vec4 base_color = vec4(0.7f, 0.8f, 1.0f, 1.0f);
    vec3 light_direction = normalize(vec3(-0.6f, -0.6f, -0.8f));

    vec3 view_direction = normalize(camera_position - fragment_position);
    vec3 reflect_direction = reflect(-light_direction, nnormal);

    float ambient = 0.3f;
    float diffuse = 0.7f * max(dot(light_direction, nnormal), 0.0f);
    float specular = 0.5f * pow(max(dot(view_direction, reflect_direction), 0.0), 32);

    fragColor = (ambient + diffuse + specular) * base_color;
}
