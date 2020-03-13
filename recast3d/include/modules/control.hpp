#include <vector>

#include "tomop/tomop.hpp"
#include "zmq.hpp"

#include "scene.hpp"
#include "scene_list.hpp"
#include "scene_module.hpp"

#include "util.hpp"

#include "graphics/components/control_component.hpp"

namespace tomovis {

using namespace tomop;

// Module for analysis, benchmarking and control
class ControlProtocol : public SceneModuleProtocol {
  public:
    std::unique_ptr<Packet> read_packet(packet_desc desc, memory_buffer& buffer,
                                        zmq::socket_t& socket,
                                        SceneList& /* scenes */) override {
        switch (desc) {
        case packet_desc::parameter_bool: {
            auto packet = std::make_unique<ParameterBoolPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return packet;
        }
        case packet_desc::parameter_float: {
            auto packet = std::make_unique<ParameterFloatPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);

            return packet;
        }
        case packet_desc::parameter_enum: {
            auto packet = std::make_unique<ParameterEnumPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);

            return packet;
        }
        case packet_desc::tracker: {
            auto packet = std::make_unique<TrackerPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);

            return packet;
        }
        case packet_desc::benchmark: {
            auto packet = std::make_unique<BenchmarkPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);

            return packet;
        }

        default: { return nullptr; }
        }
    }

    void process(SceneList& scenes, packet_desc desc,
                 std::unique_ptr<Packet> event_packet) override {

        auto get_component = [&](int scene_id) -> ControlComponent* {
            auto scene = scenes.get_scene(scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
                return nullptr;
            }
            auto control_component =
                &(ControlComponent&)scene->object().get_component("control");

            return control_component;
        };

        switch (desc) {
        case packet_desc::parameter_bool: {
            auto& packet = *(ParameterBoolPacket*)event_packet.get();
            auto control_component = get_component(packet.scene_id);
            if (!control_component) {
                return;
            }
            control_component->add_bool_parameter(packet.parameter_name,
                                                  packet.value);
            break;
        }
        case packet_desc::parameter_float: {
            auto& packet = *(ParameterFloatPacket*)event_packet.get();
            auto control_component = get_component(packet.scene_id);
            if (!control_component) {
                return;
            }
            control_component->add_float_parameter(packet.parameter_name,
                                                   packet.value);
            break;
        }
        case packet_desc::parameter_enum: {
            auto& packet = *(ParameterEnumPacket*)event_packet.get();
            auto control_component = get_component(packet.scene_id);
            if (!control_component) {
                return;
            }
            control_component->add_enum_parameter(packet.parameter_name,
                                                  packet.values);
            break;
        }
        case packet_desc::tracker: {
            auto& packet = *(TrackerPacket*)event_packet.get();
            auto control_component = get_component(packet.scene_id);
            if (!control_component) {
                return;
            }
            control_component->track_result(packet.parameter_name,
                                            packet.value);
            break;
        }
        case packet_desc::benchmark: {
            auto& packet = *(BenchmarkPacket*)event_packet.get();
            auto control_component = get_component(packet.scene_id);
            if (!control_component) {
                return;
            }
            control_component->bench_result(packet.parameter_name,
                                            packet.value);
            break;
        }

        default: { break; }
        }
    }

    std::vector<packet_desc> descriptors() override {
        return {packet_desc::parameter_bool, packet_desc::parameter_float,
                packet_desc::parameter_enum, packet_desc::tracker,
                packet_desc::benchmark};
    }
};

} // namespace tomovis
