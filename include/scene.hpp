#pragma once

#include "graphics/render_target.hpp"
#include "graphics/scene_object.hpp"


namespace tomovis {

class Scene : public RenderTarget {
  public:
    Scene();
    ~Scene();

    void render() override;

    SceneObject& object() { return *object_; }

  private:
    SceneObject* object_;
};

} // namespace tomovis
