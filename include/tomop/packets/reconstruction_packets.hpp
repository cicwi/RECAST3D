#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

class SliceDataPacket : public PacketBase<SliceDataPacket> {
   public:
    SliceDataPacket()
        : PacketBase<SliceDataPacket>(packet_desc::slice_data),
          scene_id(-1),
          slice_id(0) {}

    SliceDataPacket(int32_t scene_id_, int32_t slice_id_, std::vector<int32_t> slice_size_,
                    std::vector<uint8_t>&& data_)
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

    int32_t scene_id;
    int32_t slice_id;
    std::vector<int32_t> slice_size;
    std::vector<uint8_t> data;
};

class VolumeDataPacket : public PacketBase<VolumeDataPacket> {
   public:
    VolumeDataPacket()
        : PacketBase<VolumeDataPacket>(packet_desc::volume_data) {}

    VolumeDataPacket(int32_t scene_id_, std::vector<int32_t> volume_size_,
                     std::vector<uint8_t> data_)
        : PacketBase<VolumeDataPacket>(packet_desc::volume_data),
          scene_id(scene_id_),
          volume_size(volume_size_),
          data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | volume_size;
        buffer | data;
    }

    int32_t scene_id;
    std::vector<int32_t> volume_size;
    std::vector<uint8_t> data;
};

class SetSlicePacket : public PacketBase<SetSlicePacket> {
   public:
    SetSlicePacket()
        : PacketBase<SetSlicePacket>(packet_desc::set_slice),
          scene_id(-1),
          slice_id(0) {}

    SetSlicePacket(int32_t scene_id_, int32_t slice_id_,
                    const std::array<float, 9>& orientation_)
        : PacketBase<SetSlicePacket>(packet_desc::set_slice),
          scene_id(scene_id_),
          slice_id(slice_id_),
          orientation(orientation_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | slice_id;
        buffer | orientation;
    }

    int32_t scene_id;
    int32_t slice_id;
    std::array<float, 9> orientation;
};


class RemoveSlicePacket : public PacketBase<RemoveSlicePacket> {
   public:
    RemoveSlicePacket()
        : PacketBase<RemoveSlicePacket>(packet_desc::remove_slice),
          scene_id(-1),
          slice_id(0) {}

    RemoveSlicePacket(int32_t scene_id_, int32_t slice_id_)
        : PacketBase<RemoveSlicePacket>(packet_desc::remove_slice),
          scene_id(scene_id_),
          slice_id(slice_id_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | slice_id;
    }

    int32_t scene_id;
    int32_t slice_id;
};

}  // namespace tomop
