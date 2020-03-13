#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

struct SetPartPacket : public PacketBase<SetPartPacket> {
    static constexpr auto desc = packet_desc::set_part;
    SetPartPacket() = default;
    SetPartPacket(int32_t a, int32_t b, std::array<float, 3> c,
                  std::array<float, 3> d)
        : scene_id(a), part_id(b), min_pt(c), max_pt(d) {}
    BOOST_HANA_DEFINE_STRUCT(SetPartPacket, (int32_t, scene_id),
                             (int32_t, part_id), (std::array<float, 3>, min_pt),
                             (std::array<float, 3>, max_pt));
};

} // namespace tomop
