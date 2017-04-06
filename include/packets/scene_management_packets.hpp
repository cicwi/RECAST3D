#pragma once

#include "packets.hpp"

namespace tomop {

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

}  // namespace tomop
