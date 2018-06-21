R"(
#version 330

in vec3 beam_coord;

uniform mat4 beam_matrix;

out vec4 fragColor;

void main() {
    float intensity = 1.0f - beam_coord.z;
    fragColor = vec4(vec3(1.0f), 0.5f * intensity);
}
)"
