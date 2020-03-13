#pragma once

#include <iostream>

#include "tomop/tomop.hpp"
#include "zmq.hpp"

#include "scene.hpp"
#include "scene_list.hpp"
#include "scene_module.hpp"

#include "graphics/components/partitioning_component.hpp"

namespace tomovis {

using namespace tomop;

class PartitioningProtocol : public SceneModuleProtocol {
  public:
    std::unique_ptr<Packet> read_packet(packet_desc desc, memory_buffer& buffer,
                                        zmq::socket_t& socket,
                                        SceneList& /* scenes_ */) override {
        switch (desc) {
        case packet_desc::set_part: {
            auto packet = std::make_unique<SetPartPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return packet;
        }
        default: { return nullptr; }
        }
    }

    void process(SceneList& scenes, packet_desc desc,
                 std::unique_ptr<Packet> event_packet) override {
        switch (desc) {
        case packet_desc::set_part: {
            SetPartPacket& packet = *(SetPartPacket*)event_packet.get();

            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
                return;
            }
            auto& part_component =
                (PartitioningComponent&)scene->object().get_component(
                    "partitioning");
            auto min_pt = packet.min_pt;
            auto max_pt = packet.max_pt;
            part_component.add_part(part(packet.part_id,
                                         {min_pt[0], min_pt[1], min_pt[2]},
                                         {max_pt[0], max_pt[1], max_pt[2]}));

            break;
        }
        default: { break; }
        }
    }

    std::vector<packet_desc> descriptors() override {
        return {packet_desc::set_part};
    }
};

} // namespace tomovis
