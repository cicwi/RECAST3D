#pragma once

#include "tomop/tomop.hpp"
#include "zmq.hpp"

#include <memory>
#include <vector>

#include "scene.hpp"
#include "scene_module.hpp"

#include "graphics/components/geometry_component.hpp"
#include "graphics/components/partitioning_component.hpp"
#include "graphics/components/reconstruction_component.hpp"

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
            auto scene_id = scenes_.reserve_id();
            memcpy(reply.data(), &scene_id, sizeof(int));
            socket.send(reply);

            packet->dimension = scene_id;

            return packet;

            break;
        }

        default: { return nullptr; }
        }
    }

    void process(SceneList& scenes, packet_desc desc,
                 std::unique_ptr<Packet> event_packet) override {
        switch (desc) {
        case packet_desc::make_scene: {
            MakeScenePacket& packet = *(MakeScenePacket*)event_packet.get();
            std::cout << "Making scene: " << packet.name << " ("
                      << packet.dimension << ")\n";

            // FIXME: in the category 'here be dragons', I have hijacked the
            // dimension field to be the reserved scene_id and to always assume
            // a dimension of 3. This is because the packet originally had a
            // scene_id field, that was only filled on reception, which did not
            // make sense. However, the system we use here (delayed handling of
            // packets) makes it hard to send along arbitrary data...)
            auto scene_id = packet.dimension;

            scenes.add_scene(packet.name, scene_id, true, 3);

            auto& obj = scenes.active_scene()->object();
            obj.add_component(
                std::make_unique<ReconstructionComponent>(obj, scene_id));
            obj.add_component(
                std::make_unique<GeometryComponent>(obj, scene_id));
            obj.add_component(
                std::make_unique<PartitioningComponent>(obj, scene_id));
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
