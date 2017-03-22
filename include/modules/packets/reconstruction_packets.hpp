#pragma once

#include "packets.hpp"

namespace tomovis {

class SliceDataPacket : public PacketBase<SliceDataPacket> {
   public:
    SliceDataPacket()
        : PacketBase<SliceDataPacket>(packet_desc::slice_data),
          scene_id(-1),
          slice_id(0) {}

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

class VolumeDataPacket : public PacketBase<VolumeDataPacket> {
   public:
    VolumeDataPacket()
        : PacketBase<VolumeDataPacket>(packet_desc::volume_data) {}

    VolumeDataPacket(int scene_id_, std::vector<int> volume_size_,
                     std::vector<unsigned char>&& data_)
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

    int scene_id;
    std::vector<int> volume_size;
    std::vector<unsigned char> data;
};

class SetSlicePacket : public PacketBase<SetSlicePacket> {
   public:
    SetSlicePacket()
        : PacketBase<SetSlicePacket>(packet_desc::set_slice),
          scene_id(-1),
          slice_id(0) {}

    SetSlicePacket(int scene_id_, int slice_id_,
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

    int scene_id;
    int slice_id;
    std::array<float, 9> orientation;
};

}  // namespace tomovis
