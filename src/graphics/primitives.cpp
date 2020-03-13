#include "graphics/primitives.hpp"
namespace tomovis {

const GLfloat* line() {
    static const GLfloat data[] = {0.0f, 0.0f, 0.0f,
                                   1.0f, 1.0f, 1.0f};
    return data;
}

const GLfloat* square() {
    static const GLfloat data[] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                   1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};
    return data;
}

const GLfloat* cube() {
    static const GLfloat data[] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    return data;
}

const GLfloat* cube_wireframe() {
    static const GLfloat data[] = {
        1.0f,  1.0f,  1.0f,  // 0
        1.0f,  1.0f,  -1.0f, // 1
        1.0f,  -1.0f, -1.0f, // 2
        -1.0f, -1.0f, -1.0f, // 3
        -1.0f, -1.0f, 1.0f,  // 4
        -1.0f, 1.0f,  1.0f,  // 5
        -1.0f, 1.0f,  -1.0f, // 6
        1.0f,  -1.0f, 1.0f   // 7
    };
    return data;
}

const GLuint* cube_wireframe_idxs() {
    static const GLuint data[] = {5, 4, 5, 0, 4, 7, 7, 0, 6, 5, 1, 0,
                                   3, 4, 2, 7, 6, 3, 6, 1, 3, 2, 2, 1};
    return data;
}

const GLfloat* pyramid() {
    static const GLfloat data[] = {
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    return data;
}

const GLfloat* alt_pyramid() {
    static const GLfloat data[] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    return data;
}



} // namespace tomovis
