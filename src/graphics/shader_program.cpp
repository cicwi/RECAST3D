#include <fstream>
#include <iostream>

#include "graphics/shader_program.hpp"

namespace tomovis {

ShaderProgram::ShaderProgram(std::string vert_file, std::string frag_file) {
    auto create_shader = [](auto type, std::string file) {
        auto shader = glCreateShader(type);

        auto string_from_file = [](std::string filename) {
            std::string result;
            std::ifstream file_stream(filename);
            std::string line;
            while (std::getline(file_stream, line)) {
                result += line + "\n";
            }

            return result;
        };

        auto file_as_string = string_from_file(file);
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

void ShaderProgram::use() {
    glUseProgram(shader_program_);
}

} // namespace tomovis
