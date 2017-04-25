#version 330

in vec3 normal;
out vec4 fragColor;

void main() {
    float mult = dot(-vec3(-0.2f, -0.2f, -0.95f), normal);
    fragColor = (0.3f + 0.7f * mult) * vec4(0.8f, 0.8f, 0.8f, 1.0f);
}
