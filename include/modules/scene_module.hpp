#pragma once

#include <memory>
#include <vector>

#include "../packets.hpp"
#include "../scene_list.hpp"

namespace tomovis {

// for the 'one-way-communication' we have two parts
// a handler that knows how to read in a packet
// and an executor that knows how to execute a packet

class SceneModuleProtocol {
   public:
    virtual std::unique_ptr<Packet> read_packet(packet_desc desc,
                                                memory_buffer& buffer,
                                                zmq::socket_t& socket,
                                                SceneList& scenes_) = 0;
    virtual void process(SceneList& scenes,
                         std::unique_ptr<Packet> event_packet) = 0;

    virtual std::vector<packet_desc> descriptors() = 0;
};

}  // namespace tomovis
