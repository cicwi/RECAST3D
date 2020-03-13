#pragma once

#include <vector>
#include <utility>
#include <string>

#include <glm/glm.hpp>
#include <GL/gl3w.h>

#include "configurable.hpp"
#include "input_handler.hpp"
#include "ticker.hpp"
#include "graphics/interface/window.hpp"

namespace tomovis {

class SceneCamera : public InputHandler, public Ticker, public Window {
  public:
    SceneCamera();
    virtual ~SceneCamera(){};

    virtual glm::mat4 matrix() = 0;

    virtual void set_look_at(glm::vec3 /* center */) {}
    void tick(float) override {}

    GLuint colormap() { return colormap_texture_id_; }
    void set_colormap(int scheme);

    virtual glm::vec3& position() = 0;
    virtual glm::vec3& look_at() = 0;
    virtual void reset_view() {};

    void describe() override;

    void toggle_interaction() { interaction_disabled_ = ! interaction_disabled_; }

  protected:
    bool interaction_disabled_ = false;

  private:
    int current_scheme_ = -1;
    std::vector<std::string> schemes_;
    GLuint colormap_texture_id_;
};

} // namespace tomovis
