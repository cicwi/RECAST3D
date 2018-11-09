#pragma once

#include "tomop/tomop.hpp"
#include "zmq.hpp"

#include "scene.hpp"
#include "scene_list.hpp"
#include "scene_module.hpp"

#include "graphics/components/reconstruction_component.hpp"
#include "util.hpp"

namespace tomovis {

using namespace tomop;

// for the 'one-way-communication' we have two parts
// a handler that knows how to read in a packet
// and an executor that knows how to execute a packet

class ReconstructionProtocol : public SceneModuleProtocol {
  public:
    std::unique_ptr<tomop::Packet>
    read_packet(tomop::packet_desc desc, memory_buffer& buffer,
                zmq::socket_t& socket, SceneList& /* scenes_ */) override {
        switch (desc) {
        case packet_desc::slice_data: {
            auto packet = std::make_unique<SliceDataPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return std::move(packet);
        }

        case packet_desc::partial_slice_data: {
            auto packet = std::make_unique<PartialSliceDataPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return std::move(packet);
        }

        case packet_desc::volume_data: {
            auto packet = std::make_unique<VolumeDataPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return std::move(packet);
        }

        case packet_desc::partial_volume_data: {
            auto packet = std::make_unique<PartialVolumeDataPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return std::move(packet);
        }

        case packet_desc::group_request_slices: {
            auto packet = std::make_unique<GroupRequestSlicesPacket>();
            packet->deserialize(std::move(buffer));
            message_succes(socket);
            return std::move(packet);
        }

        default: { return nullptr; }
        }
    }

    void process(SceneList& scenes, packet_desc desc,
                 std::unique_ptr<Packet> event_packet) override {
        switch (desc) {
        case packet_desc::slice_data: {
            SliceDataPacket& packet = *(SliceDataPacket*)event_packet.get();
            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
            }
            auto& reconstruction_component =
                (ReconstructionComponent&)scene->object().get_component(
                    "reconstruction");
            reconstruction_component.set_data(packet.data, packet.slice_size,
                                              packet.slice_id, packet.additive);
            break;
        }

        case packet_desc::partial_slice_data: {
            PartialSliceDataPacket& packet =
                *(PartialSliceDataPacket*)event_packet.get();
            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
            }
            auto& reconstruction_component =
                (ReconstructionComponent&)scene->object().get_component(
                    "reconstruction");
            reconstruction_component.update_partial_slice(
                packet.data, packet.slice_offset, packet.slice_size,
                packet.global_slice_size, packet.slice_id, packet.additive);
            break;
        }

        case packet_desc::volume_data: {
            VolumeDataPacket& packet = *(VolumeDataPacket*)event_packet.get();
            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
            }
            auto& reconstruction_component =
                (ReconstructionComponent&)scene->object().get_component(
                    "reconstruction");
            reconstruction_component.set_volume_data(packet.data,
                                                     packet.volume_size);
            break;
        }

        case packet_desc::partial_volume_data: {
            PartialVolumeDataPacket& packet =
                *(PartialVolumeDataPacket*)event_packet.get();
            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
            }
            auto& reconstruction_component =
                (ReconstructionComponent&)scene->object().get_component(
                    "reconstruction");
            reconstruction_component.update_partial_volume(
                packet.data, packet.volume_offset, packet.volume_size,
                packet.global_volume_size);
            break;
        }

        case packet_desc::group_request_slices: {
            GroupRequestSlicesPacket& packet =
                *(GroupRequestSlicesPacket*)event_packet.get();
            auto scene = scenes.get_scene(packet.scene_id);
            if (!scene) {
                std::cout << "Updating non-existing scene\n";
            }

            if (group_size_requested_ < 0) {
                group_size_requested_ = packet.group_size;
                group_size_count_ = 1;
            } else {
                if (group_size_requested_ != packet.group_size) {
                    std::cout << "Group request for different group sizes "
                              << group_size_requested_
                              << " != " << packet.group_size << "\n";
                }
                group_size_count_ += 1;
            }

            if (group_size_count_ == group_size_requested_) {
                group_size_count_ = -1;
                group_size_requested_ = -1;

                auto& reconstruction_component =
                    (ReconstructionComponent&)scene->object().get_component(
                        "reconstruction");
                reconstruction_component.send_slices();
            }

            break;
        }

        default: {
            std::cout << "Reconstruction module ignoring an unknown packet..\n";
            break;
        }
        }
    }

    std::vector<packet_desc> descriptors() override {
        return {packet_desc::slice_data, packet_desc::partial_slice_data,
                packet_desc::volume_data, packet_desc::partial_volume_data,
                packet_desc::group_request_slices};
    }

  private:
    int group_size_count_ = -1;
    int group_size_requested_ = -1;
};

} // namespace tomovis
