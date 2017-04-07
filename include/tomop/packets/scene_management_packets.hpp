#pragma once

#include <cstdint>
#include <string>

#include "../packets.hpp"

namespace tomop {

class MakeScenePacket : public PacketBase<MakeScenePacket> {
   public:
    MakeScenePacket(std::string name_ = "", int32_t dimension_ = 3)
        : PacketBase<MakeScenePacket>(packet_desc::make_scene),
          name(name_),
          dimension(dimension_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | name;
        buffer | dimension;
    }

    std::string name;
    int32_t dimension;
    std::vector<float> volume_geometry;
    int32_t scene_id;
};

}  // namespace tomop
