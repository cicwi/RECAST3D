#include <iostream>
#include <string>

#include <GL/gl3w.h>

#include "graphics/components/movie/recorder.hpp"
#include <imgui.h>

namespace tomovis {

Recorder::Recorder() {}
Recorder::~Recorder() {}

void Recorder::describe() {
    ImGui::Text("Recorder");
    if (ImGui::Button("rec")) {
        start();
    }
    ImGui::SameLine();
    if (ImGui::Button("pause")) {
        recording_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("stop")) {
        stop();
    }
}

void Recorder::start() {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width_ = viewport[2];
    height_ = viewport[3];

    std::string cmd = "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s " +
                      std::to_string(width_) + "x" + std::to_string(height_) +
                      " -i - "
                      "-threads 5 -preset fast -y -c:v libx264 -pix_fmt yuv420p -crf 0 -bf 2 -vf "
                      "vflip output.mp4";

    std::cout << "Recording video...\n";

    ffmpeg_ = popen(cmd.c_str(), "w");
    if (ffmpeg_) {
        auto err_check = pclose(ffmpeg_);
        if (err_check != 0) {
            std::cout << "Child process (ffmpeg) exited with an error\n";
            return;
        } else {
            ffmpeg_ = popen(cmd.c_str(), "w");
        }
    } else {
        std::cout << "Could not open process (ffmpeg)\n";
        return;
    }

    buffer_ = new int[width_ * height_];

    recording_ = true;
}

void Recorder::stop() {
    if (ffmpeg_) {
        pclose(ffmpeg_);
        ffmpeg_ = nullptr;
    }

    if (buffer_) {
        delete buffer_;
    }

    std::cout << "Finished recording video.\n";

    recording_ = false;
}

void Recorder::capture() {
    if (!recording_) {
        return;
    }

    if (!ffmpeg_) {
        return;
    }

    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, buffer_);
    fwrite(buffer_, sizeof(int) * width_ * height_, 1, ffmpeg_);
}

} // namespace tomovis
