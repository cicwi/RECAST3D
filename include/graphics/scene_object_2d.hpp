#pragma once

#include "scene_object.hpp"


namespace tomovis {

class SceneObject2D : public SceneObject {
  public:
    SceneObject2D();
    ~SceneObject2D();

    void draw() override;

  private:
    static constexpr int size_ = 20;
    GLuint texture_id_;
};

} // namespace tomovis
