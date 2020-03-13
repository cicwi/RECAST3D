#pragma once

#include <set>

#include "input_handler.hpp"
#include "ticker.hpp"


struct GLFWwindow;

namespace tomovis {

class Input : public Ticker {
  public:
    static Input& instance(GLFWwindow* window) {
        static Input instance(window);
        return instance;
    }

    void register_handler(InputHandler& handler);
    void unregister_handler(InputHandler& handler);

    static void mouse_button_callback(GLFWwindow*, int button, int action, int);
    static void scroll_callback(GLFWwindow*, double, double yoffset);
    static void key_callback(GLFWwindow*, int key, int, int action, int mods);
    static void char_callback(GLFWwindow*, unsigned int c);

    void tick(float time_elapsed) override;

  private:
    Input(GLFWwindow* window);
    ~Input();

    Input(Input const&) = delete;
    void operator=(Input const&) = delete;

    struct InputCompare {
        bool operator()(const InputHandler* lhs, const InputHandler* rhs) const {
            return lhs->priority() < rhs->priority();
        }
    };

    std::set<InputHandler*, InputCompare> handlers_;

    GLFWwindow* window_;
};

} // namespace tomovis
