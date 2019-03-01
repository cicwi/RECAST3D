#include <fstream>
#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include "graphics/interface/interface.hpp"
#include "graphics/interface/window.hpp"

namespace tomovis {

Interface::Interface(GLFWwindow* window) {
    // Setup ImGui binding
    ImGui::CreateContext();

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see
    // extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init();

    std::string other_font = "../data/iosevka-medium.ttf";
    std::ifstream infile(other_font);
    if (infile.good()) {
        io.Fonts->AddFontFromFileTTF(other_font.c_str(), 20.0f);
    } else {
        std::cout << "Can not find Iosevka font, resorting back to default\n";
        io.Fonts->AddFontDefault();
    }
    io.MouseDrawCursor = false;

    this->set_style_();

    // prevent ini file
    io.IniFilename = "";
}

void Interface::set_style_() {
    ImGui::StyleColorsDark();
}

Interface::~Interface() { ImGui_ImplGlfw_Shutdown(); }

void Interface::render(glm::mat4) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto window : windows_) {
        window->describe();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::register_window(Window& window) { windows_.push_back(&window); }

bool Interface::handle_mouse_button(int button, bool down) {
    ImGui_ImplGlfw_MouseButtonCallback(nullptr, button,
                                       down ? GLFW_PRESS : GLFW_RELEASE, 0);
    auto io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool Interface::handle_scroll(double offset) {
    ImGui_ImplGlfw_ScrollCallback(nullptr, 0.0, offset);
    auto io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool Interface::handle_key(int key, bool down, int /* mods */) {
    ImGui_ImplGlfw_KeyCallback(nullptr, key, 0,
                               down ? GLFW_PRESS : GLFW_RELEASE, 0);
    auto io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool Interface::handle_char(unsigned int c) {
    ImGui_ImplGlfw_CharCallback(nullptr, c);
    auto io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool Interface::handle_mouse_moved(float /* x */, float /* y */) {
    auto io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

} // namespace tomovis
