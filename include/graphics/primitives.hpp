#pragma once

#include <GL/gl3w.h>

namespace tomovis {

const GLfloat* line();
const GLfloat* square();
const GLfloat* cube();
const GLfloat* cube_wireframe();
const GLuint* cube_wireframe_idxs();
const GLfloat* pyramid();
const GLfloat* alt_pyramid();

} // namespace tomovis
