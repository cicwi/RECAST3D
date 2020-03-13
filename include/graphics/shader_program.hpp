#pragma once

#include <string>

#include <GL/gl3w.h>

namespace tomovis {

class ShaderProgram {
  public:
    ShaderProgram(std::string vert, std::string frag, bool file = true);
    ~ShaderProgram();

    void use();
    GLuint handle() { return shader_program_; }

    void uniform(std::string name, glm::mat4 m);
    void uniform(std::string name, glm::vec3 v);
    void uniform(std::string name, glm::vec4 v);
    void uniform(std::string name, float x);
    void uniform(std::string name, int x);

  private:
    GLuint shader_program_;
    GLuint vert_shader_;
    GLuint frag_shader_;
};

} // namespace tomovis
