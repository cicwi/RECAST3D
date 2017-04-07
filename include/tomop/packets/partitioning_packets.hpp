#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

class SetPartPacket : public PacketBase<SetPartPacket> {
   public:
    SetPartPacket()
        : PacketBase<SetPartPacket>(packet_desc::set_part),
          scene_id(-1),
          part_id(-1) {}

    SetPartPacket(int32_t scene_id_, int32_t part_id_,
                         std::array<float, 3> min_pt_,
                         std::array<float, 3> max_pt_)
        : PacketBase<SetPartPacket>(packet_desc::set_part),
          scene_id(scene_id_),
          part_id(part_id_),
          min_pt(min_pt_),
          max_pt(max_pt_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | part_id;
        buffer | min_pt;
        buffer | max_pt;
    }

    int32_t scene_id;
    int32_t part_id;
    std::array<float, 3> min_pt;
    std::array<float, 3> max_pt;
};

}  // namespace tomop
