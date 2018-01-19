#pragma once

#include <string>

#include <zmq.hpp>

#include "exceptions.hpp"
#include "packets.hpp"

namespace tomop {

class publisher {
  public:
    publisher(std::string hostname = "tcp://localhost:5555")
        : context_(1), socket_(context_, ZMQ_REQ) {
        using namespace std::chrono_literals;

        // set socket timeout to 200 ms
        socket_.setsockopt(ZMQ_LINGER, 200);
        socket_.connect(hostname);
    }

    ~publisher() {
        socket_.close();
        context_.close();
    }

    void send(const Packet& packet) {
        packet.send(socket_);
        zmq::message_t reply;
        socket_.recv(&reply);
    }

  private:
    // publisher connection
    zmq::context_t context_;
    zmq::socket_t socket_;
};

} // namespace tomop
