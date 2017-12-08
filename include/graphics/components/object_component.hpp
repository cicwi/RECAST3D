#pragma once

#include <glm/glm.hpp>

#include "input_handler.hpp"
#include "ticker.hpp"
#include "graphics/interface/window.hpp"

namespace tomovis {

class ObjectComponent : public InputHandler, public Ticker, public Window {
   public:
    virtual void draw(glm::mat4 world_to_screen) = 0;
    virtual std::string identifier() const = 0;
    void tick(float /* time_elapsed */) override {}
    void describe() override {}

    // low priority gets drawn last
    virtual int priority() const { return 1; }
};

}  // namespace tomovis
