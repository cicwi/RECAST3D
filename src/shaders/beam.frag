#version 150

in vec2 position;

out vec4 fragColor;

void main() {
    float scale = 1.0f - min(abs(position.y), abs(position.x));
    fragColor = vec4(1.0f, 1.0f, 1.0f, 0.3f);
}
