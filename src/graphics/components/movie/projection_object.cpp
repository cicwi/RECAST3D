#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include "graphics/components/movie/projection_object.hpp"
#include "graphics/model.hpp"
#include "graphics/primitives.hpp"
#include "math_common.hpp"

namespace tomovis {

ProjectionObject::ProjectionObject() {
    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);
    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    program_ = std::make_unique<ShaderProgram>("../src/shaders/screen.vert",
                                               "../src/shaders/screen.frag");

    shadow_program_ = std::make_unique<ShaderProgram>(
        "../src/shaders/shadow_model.vert", "../src/shaders/shadow_model.frag");

    source_ = glm::vec3(0.0f, 0.0, 3.0f);
    detector_base_ = glm::vec3(-2.0f, -2.0f, -2.0f);
    detector_axis1_ = glm::vec3(4.0f, 0.0f, 0.0f);
    detector_axis2_ = glm::vec3(0.0f, 4.0f, 0.0f);

    orientation_matrix_ = create_orientation_matrix(
        detector_base_, detector_axis1_, detector_axis2_);

    beam_program_ = std::make_unique<ShaderProgram>("../src/shaders/beam.vert",
                                                    "../src/shaders/beam.frag");

    glGenVertexArrays(1, &cube_vao_handle_);
    glBindVertexArray(cube_vao_handle_);
    glGenBuffers(1, &cube_vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 9 * 12 * sizeof(GLfloat), cube(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    if (!initialize_fbo_()) {
        std::cout << "Could not initialize FBO\n";
    }
}

ProjectionObject::~ProjectionObject() {}

void ProjectionObject::describe() {
    float distance = glm::distance(source_, detector_center_());
    float new_distance = distance;
    ImGui::SliderFloat("Source distance", &new_distance, 0.1f, 10.0f);
    float relative = new_distance / distance;
    source_ = detector_center_() + ((source_ - detector_center_()) * relative);
}

bool ProjectionObject::initialize_fbo_() {
    // initialize the fbo
    glGenFramebuffers(1, &fbo_handle_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle_);

    // generate the fbo texture
    glGenTextures(1, &fbo_texture_);
    glBindTexture(GL_TEXTURE_2D, fbo_texture_);

    // set it to an empty image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_texture_, 0);

    // draw only color
    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);

    bool flag = true;
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        flag = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return flag;
}

void ProjectionObject::bind_fbo_texture_() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo_texture_);
}

glm::vec3 ProjectionObject::detector_center_() const {
    return detector_base_ + 0.5f * (detector_axis1_ + detector_axis2_);
}

void ProjectionObject::draw_tomo_(const Model& model) const {
    GLint viewport[4];
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle_);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, 1024, 1024);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    auto beam_matrix = glm::translate(glm::vec3(-1.0f)) *
                       glm::scale(glm::vec3(2.0f)) *
                       glm::inverse(beam_transform_());

    model.draw(beam_matrix, detector_base_, shadow_program_.get());

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

glm::mat4 ProjectionObject::beam_transform_() const {
    glm::mat4 transform =
        glm::translate(glm::vec3(-1.0f)) * glm::scale(glm::vec3(2.0f));
    auto near = 0.1f;
    auto dist = glm::distance(source_, detector_center_());
    auto detect_x = 0.5f * glm::length(detector_axis1_);
    auto detect_y = 0.5f * glm::length(detector_axis2_);
    auto x = (near / dist) * detect_x;
    auto y = (near / dist) * detect_y;

    glm::mat4 frust = glm::frustum(-x, x, -y, y, near, dist - near);
    return glm::translate(source_) * glm::inverse(frust) * transform;
}

void ProjectionObject::draw(glm::mat4 world_to_screen,
                            const Model& model) const {
    draw_tomo_(model);

    glEnable(GL_DEPTH_TEST);

    program_->use();

    program_->uniform("texture_sampler", 0);
    bind_fbo_texture_();

    program_->uniform("orientation_matrix", orientation_matrix_);
    program_->uniform("world_to_screen_matrix", world_to_screen);

    glBindVertexArray(vao_handle_);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    // glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
    // GL_ZERO);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto beam_to_screen = world_to_screen * beam_transform_();

    beam_program_->use();
    beam_program_->uniform("transform_matrix", beam_to_screen);
    glBindVertexArray(cube_vao_handle_);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
}

void ProjectionObject::tick(float time_elapsed) { (void)time_elapsed; }

} // namespace tomovis
