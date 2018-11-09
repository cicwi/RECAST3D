#pragma once

#include <memory>
#include <vector>

#include "tomop/tomop.hpp"

#include "../scene_list.hpp"

namespace tomovis {

using namespace tomop;

// for the 'one-way-communication' we have two parts
// a handler that knows how to read in a packet
// and an executor that knows how to execute a packet

class SceneModuleProtocol {
   public:
    virtual std::unique_ptr<tomop::Packet> read_packet(tomop::packet_desc desc,
                                                memory_buffer& buffer,
                                                zmq::socket_t& socket,
                                                SceneList& scenes_) = 0;
    virtual void process(SceneList& scenes, packet_desc desc,
                         std::unique_ptr<Packet> event_packet) = 0;

    virtual std::vector<packet_desc> descriptors() = 0;

    void message_succes(zmq::socket_t& socket) {
        zmq::message_t reply(sizeof(int));
        int success = 1;
        memcpy(reply.data(), &success, sizeof(int));
        socket.send(reply);
    }
};

}  // namespace tomovis
