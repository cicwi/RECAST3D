#pragma once

#include <GL/gl3w.h>

namespace tomovis {

const GLfloat* square();
const GLfloat* cube();
const GLfloat* cube_wireframe();
const GLuint* cube_wireframe_idxs();
const GLfloat* pyramid();

} // namespace tomovis
