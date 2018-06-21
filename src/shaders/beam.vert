R"(
#version 330

in vec3 in_position;

uniform mat4 transform_matrix;
uniform mat4 beam_matrix;

out vec3 beam_coord;

void main() {
  beam_coord = in_position;
  vec4 world_position = beam_matrix * vec4(in_position.x, in_position.y, in_position.z, 1.0f);
  gl_Position = transform_matrix * world_position;
}
)"
