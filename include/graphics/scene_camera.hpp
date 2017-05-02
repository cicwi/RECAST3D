#pragma once

#include <vector>
#include <utility>
#include <string>

#include <glm/glm.hpp>
#include <GL/gl3w.h>

#include "configurable.hpp"
#include "input_handler.hpp"
#include "ticker.hpp"

namespace tomovis {

class SceneCamera : public InputHandler, public Ticker {
  public:
    SceneCamera();
    virtual ~SceneCamera(){};

    virtual glm::mat4 matrix() = 0;
    virtual std::vector<parameter<float>>& parameters() = 0;

    virtual void set_look_at(glm::vec3 /* center */) {}
    void tick(float) override {}

    GLuint colormap() { return colormap_texture_id_; }
    void set_colormap(std::string name);

    virtual glm::vec3& position() = 0;
    virtual glm::vec3& look_at() = 0;

    void toggle_interaction() { interaction_disabled_ = ! interaction_disabled_; }

  protected:
    bool interaction_disabled_ = false;

  private:
    GLuint colormap_texture_id_;
};

} // namespace tomovis
