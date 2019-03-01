#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "object_component.hpp"

namespace tomovis {

class SceneObject;

class ControlComponent : public ObjectComponent {
  public:
    ControlComponent(SceneObject& object, int scene_id);
    std::string identifier() const override { return "control"; }
    void describe() override;

    void add_float_parameter(std::string name, float value);
    void add_bool_parameter(std::string name, bool value);
    void add_enum_parameter(std::string name, std::vector<std::string> values);

    void bench_result(std::string name, float value);
    void track_result(std::string name, float value);

  private:
    void describe_parameters_();
    void describe_trackers_();
    void describe_benchmarks_();

    // Note: scene objects are publishers, `object_->send(packet)` for parameter
    // updates
    SceneObject& object_;
    int scene_id_;

    std::map<std::string, std::pair<float, std::unique_ptr<char[]>>>
    float_parameters_;
  std::map<std::string, bool> bool_parameters_;
    std::map<std::string, std::pair<std::vector<std::string>, std::string>>
        enum_parameters_;

    std::map<std::string, std::vector<float>> trackers_;
    std::map<std::string, std::vector<float>> benchmarks_;
};

} // namespace tomovis
