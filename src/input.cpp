#include "input.hpp"
#include "input_handler.hpp"

#include <GLFW/glfw3.h>

namespace tomovis {

Input::Input(GLFWwindow* window) {
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
}

Input::~Input() {}

void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    for (auto& handler : instance(window).handlers_) {
        if (handler->handle_mouse_button(button, action))
            return;
    }
}

void Input::scroll_callback(GLFWwindow* window, double, double yoffset) {
    for (auto& handler : instance(window).handlers_) {
        if (handler->handle_scroll(yoffset))
            return;
    }
}

void Input::key_callback(GLFWwindow* window, int key, int, int action, int mods) {
    for (auto& handler : instance(window).handlers_) {
        if (handler->handle_key(key, action, mods))
            return;
    }
}

void Input::char_callback(GLFWwindow* window, unsigned int c) {
    for (auto& handler : instance(window).handlers_) {
        if (handler->handle_char(c))
            return;
    }
}

void Input::register_handler(InputHandler& handler) {
    handlers_.insert(&handler);
}

} // namespace tomovis
