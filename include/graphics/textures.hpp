#pragma once

#include <vector>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

namespace tomovis {

template <typename T = unsigned char>
class texture {
   public:
    texture(int x, int y) : x_(x), y_(y) {
        glGenTextures(1, &texture_id_);
        std::vector<T> data(x * y);
        for (int i = 0; i < x * y; ++i) {
            data[i] = 255 * ((i + i / x) % 2);
        }
        fill_texture(data);
    }

    texture(int x, int y, std::vector<T>& data) {
        glGenTextures(1, &texture_id_);
        fill_texture(data);
    }

    ~texture() { glDeleteTextures(1, &texture_id_); }

    void set_data(std::vector<T>& data, int x, int y) {
        x_ = x;
        y_ = y;
        fill_texture(data);
    }

    void fill_texture(std::vector<T>& data) {
        glBindTexture(GL_TEXTURE_2D, texture_id_);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // FIXME GL_UNSIGNED_BYTE depends on T
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x_, y_, 0, GL_RED,
                     GL_UNSIGNED_BYTE, data.data());

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bind() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
    }

    static void unbind() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

   private:
    GLuint texture_id_;
    int x_ = -1;
    int y_ = -1;
};

}  // namespace tomovis
