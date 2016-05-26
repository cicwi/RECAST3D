#pragma once

#include "window.hpp"


namespace tomovis {

class Scene;

class SceneControl : public Window {
  public:
    SceneControl(Scene& scene);
    ~SceneControl();

    void describe() override;

  private:
    Scene& scene_;
};

} // namespace tomovis
