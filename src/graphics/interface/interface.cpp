#include <imgui.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_glfw_gl3.h"

#include "graphics/interface/interface.hpp"
#include "graphics/interface/window.hpp"


namespace tomovis {

Interface::Interface(GLFWwindow* window) {
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/TTF/Hack-Regular.ttf", 15.0f);
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

} // namespace tomovis
