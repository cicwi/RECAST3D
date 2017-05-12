#pragma once

#include <iostream>

#include "zmq.hpp"

#include "../serialize.hpp"
#include "scene.hpp"
#include "scene_list.hpp"
#include "scene_module.hpp"

#include "graphics/components/geometry_component.hpp"
#include "graphics/components/reconstruction_component.hpp"
#include "modules/packets/geometry_packets.hpp"

namespace tomovis {

class GeometryProtocol : public SceneModuleProtocol {
  public:
    std::unique_ptr<Packet> read_packet(packet_desc desc, memory_buffer& buffer,
                                        zmq::socket_t& socket,
                                        SceneList& /* scenes_ */) override {
        switch (desc) {
        case packet_desc::geometry_specification: {
            auto packet = std::make_unique<GeometrySpecificationPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return std::move(packet);
        }

        case packet_desc::projection_data: {
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
            GeometrySpecificationPacket& packet =
                *(GeometrySpecificationPacket*)event_packet.get();

            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
                return;
            }
            auto& recon_component =
                (ReconstructionComponent&)scene->object().get_component(
                    "reconstruction");
            auto min_pt = packet.volume_min_point;
            auto max_pt = packet.volume_max_point;
            recon_component.set_volume_position(
                {min_pt[0], min_pt[1], min_pt[2]},
                {max_pt[0], max_pt[1], max_pt[2]});

            break;
        }

        case packet_desc::projection_data: {
            ProjectionDataPacket& packet =
                *(ProjectionDataPacket*)event_packet.get();

            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
                return;
            }
            auto& geometry_component =
                (GeometryComponent&)scene->object().get_component("geometry");

            auto xs = packet.detector_orientation;

            projection proj(packet.projection_id);
            proj.source_position = {packet.source_position[0],
                                    packet.source_position[1],
                                    packet.source_position[2]};
            proj.set_orientation({xs[6], xs[7], xs[8]}, {xs[0], xs[1], xs[2]},
                                 {xs[3], xs[4], xs[5]});
            proj.parallel = false;
            proj.data_texture.set_data(packet.data, packet.detector_pixels[0],
                                       packet.detector_pixels[1]);

            geometry_component.add_projection(std::move(proj));
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

} // namespace tomovis
