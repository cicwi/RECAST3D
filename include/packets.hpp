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
    update_image,
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
    MakeScenePacket(std::string name_ = "")
        : PacketBase<MakeScenePacket>(packet_desc::make_scene), name(name_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | name;
    }

    std::string name;
    int scene_id;
};

class UpdateImagePacket : public PacketBase<UpdateImagePacket> {
   public:
    UpdateImagePacket()
        : PacketBase<UpdateImagePacket>(packet_desc::update_image),
          scene_id(-1) {}

    UpdateImagePacket(int scene_id_, std::vector<int> image_size_,
                      std::vector<unsigned char>&& data_)
        : PacketBase<UpdateImagePacket>(packet_desc::update_image),
          scene_id(scene_id_),
          image_size(image_size_),
          data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | image_size;
        buffer | data;
    }

    int scene_id;
    std::vector<int> image_size;
    std::vector<unsigned char> data;
};

}  // namespace tomovis
