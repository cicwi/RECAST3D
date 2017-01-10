#pragma once

#include <zmq.hpp>

#include <vector>

#include "serialize.hpp"

namespace tomovis {

enum class packet_desc : int {
    // making a scene is done by supplying a name and some
    // information on the scene, e.g.
    // 2d or 3d, or dimensions
    // the scene_id will be returned
    make_scene,
    slice_data,
};

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

    std::size_t size() {
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

class MakeScenePacket : public PacketBase<MakeScenePacket> {
   public:
    MakeScenePacket(std::string name_ = "", int dimension_ = 3)
        : PacketBase<MakeScenePacket>(packet_desc::make_scene),
          name(name_),
          dimension(dimension_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | name;
        buffer | dimension;
    }

    std::string name;
    int dimension;
    std::vector<float> volume_geometry;
    int scene_id;
};

class SliceDataPacket : public PacketBase<SliceDataPacket> {
   public:
    SliceDataPacket()
        : PacketBase<SliceDataPacket>(packet_desc::slice_data),
          scene_id(-1), slice_id(0) {}

    SliceDataPacket(int scene_id_, int slice_id_, std::vector<int> slice_size_,
                      std::vector<unsigned char>&& data_)
        : PacketBase<SliceDataPacket>(packet_desc::slice_data),
          scene_id(scene_id_),
          slice_id(slice_id_),
          slice_size(slice_size_),
          data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | slice_id;
        buffer | slice_size;
        buffer | data;
    }

    int scene_id;
    int slice_id;
    std::vector<int> slice_size;
    std::vector<unsigned char> data;
};

}  // namespace tomovis
