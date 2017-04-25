#version 330

in vec3 normal;
out vec4 fragColor;

void main() {
    vec4 base_color = vec4(0.8f, 0.8f, 0.8f, 1.0f);
    float ambient = 0.3f;
    float diffuse = 0.7f * max(dot(-vec3(-0.2f, -0.2f, -0.95f), normal), 0.0f);
    fragColor = (ambient + diffuse) * base_color;
}
