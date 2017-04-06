#pragma once

#include <zmq.hpp>

#include <vector>

#include "descriptors.hpp"
#include "serialize.hpp"

namespace tomop {

class Packet {
   public:
    Packet(packet_desc desc_) : desc(desc_) {}
    packet_desc desc;

    struct omembuf {
        omembuf(memory_buffer& membuf_) : membuf(membuf_) {}

        template <typename T>
        void operator|(T& rhs) {
            membuf >> rhs;
        }

        memory_buffer& membuf;
    };

    struct imembuf {
        imembuf(memory_buffer& membuf_) : membuf(membuf_) {}

        template <typename T>
        void operator|(T& rhs) {
            membuf << rhs;
        }

        memory_buffer& membuf;
    };

    virtual std::size_t size() = 0;
    virtual memory_buffer serialize(int size) = 0;
    virtual void deserialize(memory_buffer buffer) = 0;

    virtual ~Packet() = default;
};

template <class Derived>
class PacketBase : public Packet {
   public:
    PacketBase(packet_desc desc_) : Packet(desc_) {}

    void send(zmq::socket_t& socket) {
        auto packet_size = size();
        zmq::message_t request(packet_size);
        memcpy(request.data(), &serialize(packet_size).buffer[0], packet_size);
        socket.send(request);
    }

    std::size_t size() override {
        scale total;
        total | this->desc;
        ((Derived*)this)->fill(total);
        return total.size;
    }

    memory_buffer serialize(int packet_size = -1) override {
        if (packet_size == -1) {
            packet_size = size();
        }
        memory_buffer buffer(packet_size);
        buffer << this->desc;

        auto im = imembuf(buffer);
        ((Derived*)this)->fill(im);

        return buffer;
    }

    void deserialize(memory_buffer buffer) override {
        buffer >> this->desc;
        auto om = omembuf(buffer);
        ((Derived*)this)->fill(om);
    }
};

}  // namespace tomop
