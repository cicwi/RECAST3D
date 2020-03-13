R"(
#version 150

uniform vec3 rgb_color;
out vec4 fragColor;

void main() {
    fragColor = vec4(rgb_color, 1.0f);
}
)"
