#pragma once

#include "window.hpp"


namespace tomovis {

class SceneList;

class SceneControl : public Window {
  public:
    SceneControl(SceneList& scenes);
    ~SceneControl();

    void describe() override;

  private:
    SceneList& scenes_;
};

} // namespace tomovis
