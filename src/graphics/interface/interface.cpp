#include <imgui.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_glfw_gl3.h"

#include "graphics/interface/interface.hpp"
#include "graphics/interface/window.hpp"


namespace tomovis {

Interface::Interface(GLFWwindow* window) {
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, false);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../data/iosevka-medium.ttf", 20.0f);
    io.MouseDrawCursor = false;

    // prevent ini file
    io.IniFilename = "";
}

Interface::~Interface() {
    ImGui_ImplGlfwGL3_Shutdown();
}

void Interface::render(glm::mat4) {;
    ImGui_ImplGlfwGL3_NewFrame();

    for (auto window : windows_) {
        window->describe();
    }

    ImGui::Render();
}

void Interface::register_window(Window& window) {
    windows_.push_back(&window);
}

bool Interface::handle_mouse_button(int button, bool down) {
    ImGui_ImplGlfwGL3_MouseButtonCallback(nullptr, button,
                                          down ? GLFW_PRESS : GLFW_RELEASE, 0);
    auto io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool Interface::handle_scroll(double offset) {
    ImGui_ImplGlfwGL3_ScrollCallback(nullptr, 0.0, offset);
    auto io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool Interface::handle_key(int key, bool down, int /* mods */) {
    ImGui_ImplGlfwGL3_KeyCallback(nullptr, key, 0,
                                  down ? GLFW_PRESS : GLFW_RELEASE, 0);
    auto io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool Interface::handle_char(unsigned int c) {
    ImGui_ImplGlfwGL3_CharCallback(nullptr, c);
    auto io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool Interface::handle_mouse_moved(float /* x */, float /* y */) {
    auto io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

} // namespace tomovis
