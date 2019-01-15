#pragma once

#include <cstdint>
#include <string>

#include "../packets.hpp"

namespace tomop {

struct MakeScenePacket : public PacketBase<MakeScenePacket> {
    static constexpr auto desc = packet_desc::make_scene;
    MakeScenePacket() = default;
    MakeScenePacket(std::string a, int32_t b) : name(a), dimension(b) {}
    BOOST_HANA_DEFINE_STRUCT(MakeScenePacket, (std::string, name),
                             (int32_t, dimension));
};

struct KillScenePacket : public PacketBase<KillScenePacket> {
    static constexpr auto desc = packet_desc::kill_scene;
    KillScenePacket() = default;
    KillScenePacket(int32_t a) : scene_id(a) {}
    BOOST_HANA_DEFINE_STRUCT(KillScenePacket, (int32_t, scene_id));
};

} // namespace tomop
