#version 330

in vec3 beam_coord;

uniform vec3 camera_position;
uniform mat4 beam_matrix;

out vec4 fragColor;

void main() {
    float intensity = 1.0f - beam_coord.z;
    vec3 normal = normalize(vec3(2.0f * beam_coord.xy - vec2(1.0f), 0.0f));
    vec4 world_position = beam_matrix * vec4(normal.xy, beam_coord.z, 0.0f);
    vec3 look_direction = vec3(normalize(world_position.xy - camera_position.xy), 0.0f);
    float fall_off = pow(dot(look_direction, normal), 1.5f);

    fragColor = vec4(vec3(1.0f), 0.5f * intensity);
}
