#include <fstream>
#include <iostream>

#include <glm/glm.hpp>

#include "graphics/shader_program.hpp"

namespace tomovis {

ShaderProgram::ShaderProgram(std::string vert_file, std::string frag_file, bool from_file) {
    auto create_shader = [from_file](auto type, std::string file) {
        auto shader = glCreateShader(type);

        auto string_from_file = [](std::string filename) {
            std::ifstream file_stream(filename);
            if(!file_stream.good()) {
                std::cerr << "Shader: " << filename << " not found\n";
                exit(-1);
            }
            std::string result((std::istreambuf_iterator<char>(file_stream)),
                               std::istreambuf_iterator<char>());
            return result;
        };

        auto file_as_string = from_file ? string_from_file(file) : file;
        auto file_buffer = file_as_string.c_str();

        GLint file_size = file_as_string.size() + 1;
        glShaderSource(shader, 1, (const GLchar**)&file_buffer, &file_size);

        glCompileShader(shader);

        GLint did_compile = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &did_compile);
        if (!did_compile) {
            int error_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_length);
            auto log = (char*)malloc(error_length);
            glGetShaderInfoLog(shader, error_length, &error_length, log);
            std::cout << log << "\n";
            free(log);
            throw;
            return shader;
        }

        return shader;
    };

    vert_shader_ = create_shader(GL_VERTEX_SHADER, vert_file);
    frag_shader_ = create_shader(GL_FRAGMENT_SHADER, frag_file);

    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vert_shader_);
    glAttachShader(shader_program_, frag_shader_);

    glBindAttribLocation(shader_program_, 0, "in_position");
    glBindAttribLocation(shader_program_, 1, "in_color");

    glLinkProgram(shader_program_);

    GLint did_link = 0;
    glGetProgramiv(shader_program_, GL_LINK_STATUS, (GLint*)&did_link);
    if (did_link == 0) {
        int max_length;
        glGetProgramiv(shader_program_, GL_INFO_LOG_LENGTH, &max_length);

        auto log = (char*)malloc(max_length);

        glGetProgramInfoLog(shader_program_, max_length, &max_length, log);

        free(log);
        throw;
    }
}

ShaderProgram::~ShaderProgram() {
    glDetachShader(shader_program_, vert_shader_);
    glDetachShader(shader_program_, frag_shader_);
    glDeleteProgram(shader_program_);
    glDeleteShader(vert_shader_);
    glDeleteShader(frag_shader_);
}

void ShaderProgram::use() { glUseProgram(shader_program_); }

void ShaderProgram::uniform(std::string name, glm::mat4 m) {
    glUniformMatrix4fv(glGetUniformLocation(handle(), name.c_str()), 1,
                       GL_FALSE, &m[0][0]);
}

void ShaderProgram::uniform(std::string name, glm::vec3 v) {
    glUniform3fv(glGetUniformLocation(handle(), name.c_str()), 1, &v[0]);
}

void ShaderProgram::uniform(std::string name, glm::vec4 v) {
    glUniform4fv(glGetUniformLocation(handle(), name.c_str()), 1, &v[0]);
}

void ShaderProgram::uniform(std::string name, float x) {
    glUniform1f(glGetUniformLocation(handle(), name.c_str()), x);
}

void ShaderProgram::uniform(std::string name, int x) {
    glUniform1i(glGetUniformLocation(handle(), name.c_str()), x);
}

} // namespace tomovis
