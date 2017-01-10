#include <memory>

#include <GL/gl3w.h>

#include "graphics/scene_object.hpp"
#include "graphics/shader_program.hpp"

#include "graphics/scene_camera.hpp"

namespace tomovis {

SceneObject::SceneObject() {}

SceneObject::~SceneObject() {
    glDeleteVertexArrays(1, &vao_handle_);
    glDeleteBuffers(1, &vbo_handle_);
}

}  // namespace tomovis
