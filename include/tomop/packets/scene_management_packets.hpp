#pragma once

#include <cstdint>
#include <string>

#include "../packets.hpp"

namespace tomop {

class MakeScenePacket : public PacketBase<MakeScenePacket> {
  public:
    MakeScenePacket(std::string name_ = "", int32_t dimension_ = 3)
        : PacketBase<MakeScenePacket>(packet_desc::make_scene), name(name_),
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

class KillScenePacket : public PacketBase<KillScenePacket> {
  public:
    KillScenePacket()
        : PacketBase<KillScenePacket>(packet_desc::kill_scene), scene_id(-1) {}

    KillScenePacket(int scene_id_)
        : PacketBase<KillScenePacket>(packet_desc::kill_scene),
          scene_id(scene_id_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
    }

    int32_t scene_id;
};

} // namespace tomop
