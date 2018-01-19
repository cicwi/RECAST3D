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

    template <typename BufferT>
    struct omembuf {
        omembuf(BufferT& membuf_) : membuf(membuf_) {}

        template <typename T>
        void operator|(T& rhs) {
            membuf >> rhs;
        }

        BufferT& membuf;
    };

    template <typename BufferT>
    struct imembuf {
        imembuf(BufferT& membuf_) : membuf(membuf_) {}

        template <typename T>
        void operator|(T& rhs) {
            membuf << rhs;
        }

        BufferT& membuf;
    };

    void send(zmq::socket_t& socket) const {
        auto packet_size = size();
        zmq::message_t request(packet_size);
        serialize(request);
        socket.send(request);
    }

    virtual std::size_t size() const = 0;
    virtual memory_buffer serialize(int size) const = 0;
    virtual void deserialize(memory_buffer buffer) = 0;

    virtual void serialize(zmq::message_t& request) const = 0;
    virtual void deserialize(zmq::message_t& request) = 0;

    virtual ~Packet() = default;
};

template <class Derived>
class PacketBase : public Packet {
   public:
    PacketBase(packet_desc desc_) : Packet(desc_) {}


    std::size_t size() const override {
        scale total;
        total | this->desc;
        ((Derived*)this)->fill(total);
        return total.size;
    }

    memory_buffer serialize(int packet_size = -1) const override {
        if (packet_size == -1) {
            packet_size = size();
        }
        memory_buffer buffer(packet_size);
        buffer << this->desc;

        auto im = imembuf<memory_buffer>(buffer);
        ((Derived*)this)->fill(im);

        return buffer;
    }

    void deserialize(memory_buffer buffer) override {
        buffer >> this->desc;
        auto om = omembuf<memory_buffer>(buffer);
        ((Derived*)this)->fill(om);
    }

    void serialize(zmq::message_t& request) const override {
        memory_span buffer(request.size(), (char*)request.data());
        buffer << this->desc;

        auto im = imembuf<memory_span>(buffer);
        ((Derived*)this)->fill(im);
    }

    void deserialize(zmq::message_t& request) override {
        memory_span buffer(request.size(), (char*)request.data());
        buffer >> this->desc;

        auto om = omembuf<memory_span>(buffer);
        ((Derived*)this)->fill(om);
    }
};

}  // namespace tomop
