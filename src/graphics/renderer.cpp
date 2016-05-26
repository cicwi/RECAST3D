#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "graphics/renderer.hpp"

namespace tomovis {

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

Renderer::Renderer() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        throw;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(1280, 720, "TomoVis", NULL, NULL);
    glfwMakeContextCurrent(window_);
    gl3wInit();
}

Renderer::~Renderer() {
    glfwTerminate();
}

void Renderer::main_loop() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto target : targets_) {
            target->render();
        }

        glfwSwapBuffers(window_);
    }
}


void Renderer::register_target(RenderTarget& target) {
    targets_.insert(&target);
}

} // namespace tomovis
