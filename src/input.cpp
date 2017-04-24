#include <algorithm>

#include "input.hpp"
#include "input_handler.hpp"

#include <GLFW/glfw3.h>

namespace tomovis {

Input::Input(GLFWwindow* window) : window_(window) {
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

void Input::tick(float /* time_elapsed */) {
    double mouse_x = 0.0;
    double mouse_y = 0.0;
    glfwGetCursorPos(window_, &mouse_x, &mouse_y);

    int w = 0;
    int h = 0;
    glfwGetWindowSize(window_, &w, &h);
    mouse_x = (2.0 * (mouse_x / w) - 1.0) * ((float)w / h);
    mouse_y = 2.0 * (mouse_y / h) - 1.0;

    for (auto& handler : instance(window_).handlers_) {
        if (handler->handle_mouse_moved(mouse_x, mouse_y))
            return;
    }
}

void Input::register_handler(InputHandler& handler) {
    handlers_.insert(&handler);
}

void Input::unregister_handler(InputHandler& handler) {
    handlers_.erase(std::find(handlers_.begin(), handlers_.end(), &handler));
}

} // namespace tomovis
