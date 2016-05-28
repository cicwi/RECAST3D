#pragma once

#include "graphics/render_target.hpp"
#include "graphics/scene_object.hpp"


namespace tomovis {

class Scene : public RenderTarget {
  public:
    Scene(std::string name);
    ~Scene();

    void render(glm::mat4 window_matrix) override;

    SceneObject& object() { return *object_; }

    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = name; }

  private:
    SceneObject* object_;
    std::string name_;
};

} // namespace tomovis
