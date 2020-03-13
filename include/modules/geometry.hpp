#pragma once

#include <algorithm>
#include <iostream>

#include "tomop/tomop.hpp"
#include "zmq.hpp"

#include "scene.hpp"
#include "scene_list.hpp"
#include "scene_module.hpp"
#include "util.hpp"

#include "graphics/components/geometry_component.hpp"
#include "graphics/components/reconstruction_component.hpp"

namespace tomovis {

using namespace tomop;

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
            return packet;
        }

        case packet_desc::projection_data: {
            auto packet = std::make_unique<ProjectionDataPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return packet;
        }

        case packet_desc::partial_projection_data: {
            auto packet = std::make_unique<PartialProjectionDataPacket>();
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
            proj.data = std::move(packet.data);
            proj.size = packet.detector_pixels;
            proj.update_texture();

            geometry_component.push_projection(std::move(proj));
            break;
        }

        case packet_desc::partial_projection_data: {
            PartialProjectionDataPacket& packet =
                *(PartialProjectionDataPacket*)event_packet.get();

            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
                return;
            }
            auto& geometry_component =
                (GeometryComponent&)scene->object().get_component("geometry");

            auto& proj =
                geometry_component.get_projection(packet.projection_id);

            proj.source_position = {packet.source_position[0],
                                    packet.source_position[1],
                                    packet.source_position[2]};
            auto xs = packet.detector_orientation;
            proj.set_orientation({xs[6], xs[7], xs[8]}, {xs[0], xs[1], xs[2]},
                                 {xs[3], xs[4], xs[5]});
            proj.parallel = false;
            if (proj.contributions == 0) {
                proj.data.resize(packet.detector_pixels[0] *
                                 packet.detector_pixels[1]);
                std::fill(proj.data.begin(), proj.data.end(), 0.0f);
                proj.size = packet.detector_pixels;
            }

            int idx = 0;
            auto po = packet.partial_offset;
            auto ps = packet.partial_size;
            for (int j = po[1]; j < po[1] + ps[1]; ++j) {
                for (int i = po[0]; i < po[0] + ps[0]; ++i) {
                    proj.data[j * proj.size[0] + i] += packet.data[idx++];
                }
            }
            proj.contributions++;

            proj.update_texture();

            break;
        }

        default: { break; }
        }
    }

    std::vector<packet_desc> descriptors() override {
        return {packet_desc::geometry_specification,
                packet_desc::projection_data,
                packet_desc::partial_projection_data};
    }
};

} // namespace tomovis
