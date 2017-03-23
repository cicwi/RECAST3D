#pragma once

#include <iostream>

#include "zmq.hpp"

#include "../serialize.hpp"
#include "scene.hpp"
#include "scene_list.hpp"
#include "scene_module.hpp"

#include "graphics/components/geometry_component.hpp"
#include "modules/packets/geometry_packets.hpp"

namespace tomovis {

class GeometryProtocol : public SceneModuleProtocol {
   public:
    std::unique_ptr<Packet> read_packet(packet_desc desc, memory_buffer& buffer,
                                        zmq::socket_t& socket,
                                        SceneList& scenes_) override {
        switch (desc) {
            case packet_desc::geometry_specification: {
                std::cout << "Received geo spec\n";
                auto packet = std::make_unique<GeometrySpecificationPacket>();
                packet->deserialize(std::move(buffer));
                message_succes(socket);
                return std::move(packet);
            }

            case packet_desc::projection_data: {
                std::cout << "Received projection data\n";
                auto packet = std::make_unique<ProjectionDataPacket>();
                packet->deserialize(std::move(buffer));
                message_succes(socket);
                return std::move(packet);
            }
            default: { return nullptr; }
        }
    }

    void process(SceneList& scenes,
                 std::unique_ptr<Packet> event_packet) override {
        switch (event_packet->desc) {
            case packet_desc::geometry_specification: {
                GeometrySpecificationPacket& packet = *(GeometrySpecificationPacket*)event_packet.get();
                break;
            }

            case packet_desc::projection_data: {
                ProjectionDataPacket& packet = *(ProjectionDataPacket*)event_packet.get();
                break;
            }

            default: { break; }
        }
    }

    std::vector<packet_desc> descriptors() override {
        return {packet_desc::geometry_specification,
                packet_desc::projection_data};
    }
};

}  // namespace tomovis
