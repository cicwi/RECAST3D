#version 150

in vec3 position;

out vec4 fragColor;

void main() {
    float scale = 1.0f - 0.5f * position.z;
    fragColor = vec4(vec3(1.0f), 0.3f);
}
