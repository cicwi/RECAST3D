#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/material.hpp"
#include "graphics/interface/window.hpp"
#include "ticker.hpp"

namespace tomovis {

class MovieComponent;

class Animation {
  public:
    Animation(float at, float duration);
    virtual void update(float time) = 0;

  protected:
    float at_;
    float duration_;
};

float mix(float x, float y, float a);
glm::vec3 mix(glm::vec3 x, glm::vec3 y, float a);
Material mix(Material x, Material y, float a);

template <typename T>
class PropertyAnimation : public Animation {
  public:
    PropertyAnimation(float at, float duration, T begin, T end, T& target)
        : Animation(at, duration), begin_(begin), end_(end), target_(target) {}

    void update(float time) override {
        if (time < at_ || time > at_ + duration_) {
            return;
        }

        float alpha = (time - at_) / duration_;
        target_ = mix(begin_, end_, alpha);
    }

  private:
    T begin_;
    T end_;
    T& target_;
};

class Storyboard : public Window, public Ticker {
  public:
    Storyboard(MovieComponent* movie);
    ~Storyboard();

    void describe() override;
    void tick(float time_elapsed) override;

  private:
    void script_();

    bool running_ = false;
    float t_ = 0.0f;

    std::vector<std::unique_ptr<Animation>> animations_;

    MovieComponent* movie_;
};

} // namespace tomovis
