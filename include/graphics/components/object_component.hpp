#pragma once

namespace tomovis {

class ObjectComponent {
   public:
    virtual void draw(glm::mat4 world_to_screen) const = 0;
    virtual std::string identifier() const = 0;
};

}  // namespace tomovis
