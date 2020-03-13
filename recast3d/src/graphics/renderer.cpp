#include <algorithm>
#include <stdio.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>

#include "graphics/renderer.hpp"

namespace tomovis {

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error %d: %s\n", error, description);
}

Renderer::Renderer() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        throw;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window_ = glfwCreateWindow(1920, 1080, "RECAST3D", NULL, NULL);
    glfwMakeContextCurrent(window_);

    gl3wInit();
    glEnable(GL_MULTISAMPLE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    previous_time_ = glfwGetTime();
}

Renderer::~Renderer() { glfwTerminate(); }

void Renderer::main_loop() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        auto current_time = glfwGetTime();
        float time_elapsed = current_time - previous_time_;
        previous_time_ = current_time;

        const auto time_step = 0.0166666666;
        for (auto ticker : tickers_) {
            while (time_elapsed > time_step) {
                ticker->tick(time_step);
                time_elapsed -= time_step;
            }
            ticker->tick(time_elapsed);
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float ratio = (float)display_h / (float)display_w;

        auto window_matrix = glm::scale(glm::vec3(ratio, 1.0, 1.0));

        for (auto target : targets_) {
            target->render(window_matrix);
        }

        glfwSwapBuffers(window_);
    }
}

void Renderer::register_target(RenderTarget& target) {
    targets_.insert(&target);
}

void Renderer::register_ticker(Ticker& ticker) { tickers_.push_back(&ticker); }

void Renderer::unregister_target(RenderTarget& target) {
    targets_.erase(std::find(targets_.begin(), targets_.end(), &target));
}

void Renderer::unregister_ticker(Ticker& ticker) {
    tickers_.erase(std::find(tickers_.begin(), tickers_.end(), &ticker));
}

} // namespace tomovis
