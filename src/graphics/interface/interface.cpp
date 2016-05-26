#include <imgui.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_glfw_gl3.h"

#include "graphics/interface/interface.hpp"


namespace tomovis {

Interface::Interface(GLFWwindow* window) {
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/TTF/Hack-Regular.ttf", 15.0f);

    // prevent ini file
    io.IniFilename = "";
}

Interface::~Interface() {
    ImGui_ImplGlfwGL3_Shutdown();
}

void Interface::render() {;
    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    ImGui_ImplGlfwGL3_NewFrame();

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears
    // in a window automatically called "Debug"
    {
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        if (ImGui::Button("Test Window"))
            show_test_window ^= 1;
        if (ImGui::Button("Another Window"))
            show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window, this time using an explicit Begin/End
    // pair
    if (show_another_window) {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in
    // ImGui::ShowTestWindow()
    if (show_test_window) {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);
    }

    ImGui::Render();
}

} // namespace tomovis
