#pragma once

#include "zmq.hpp"
#include "tomop/tomop.hpp"

#include <memory>
#include <vector>

#include "scene.hpp"
#include "scene_module.hpp"

#include "graphics/components/reconstruction_component.hpp"
#include "graphics/components/geometry_component.hpp"
#include "graphics/components/partitioning_component.hpp"

namespace tomovis {

// for the 'one-way-communication' we have two parts
// a handler that knows how to read in a packet
// and an executor that knows how to execute a packet

class ManageSceneProtocol : public SceneModuleProtocol {
  public:
    std::unique_ptr<Packet> read_packet(packet_desc desc, memory_buffer& buffer,
                                        zmq::socket_t& socket,
                                        SceneList& scenes_) override {
        switch (desc) {
        case packet_desc::make_scene: {
            zmq::message_t reply(sizeof(int));

            auto packet = std::make_unique<MakeScenePacket>();
            packet->deserialize(std::move(buffer));

            // reserve id from scenes_ and return it
            packet->scene_id = scenes_.reserve_id();
            memcpy(reply.data(), &packet->scene_id, sizeof(int));
            socket.send(reply);

            return std::move(packet);

            break;
        }

        default: { return nullptr; }
        }
    }

    void process(SceneList& scenes,
                 std::unique_ptr<Packet> event_packet) override {
        switch (event_packet->desc) {
        case packet_desc::make_scene: {
            MakeScenePacket& packet = *(MakeScenePacket*)event_packet.get();
            std::cout << "Making scene: " << packet.name << "\n";
            scenes.add_scene(packet.name, packet.scene_id, true,
                             packet.dimension);
            auto& obj = scenes.active_scene()->object();
            obj.add_component(std::make_unique<ReconstructionComponent>(
                obj, packet.scene_id));
            obj.add_component(
                std::make_unique<GeometryComponent>(obj, packet.scene_id));
            obj.add_component(
                std::make_unique<PartitioningComponent>(obj, packet.scene_id));
            break;
        }

        default: { break; }
        }
    }

    std::vector<packet_desc> descriptors() override {
        return {packet_desc::make_scene};
    }
};

} // namespace tomovis
