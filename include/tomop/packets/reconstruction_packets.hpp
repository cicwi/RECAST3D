#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

class SliceDataPacket : public PacketBase<SliceDataPacket> {
  public:
    SliceDataPacket()
        : PacketBase<SliceDataPacket>(packet_desc::slice_data), scene_id(-1),
          slice_id(0) {}

    SliceDataPacket(int32_t scene_id_, int32_t slice_id_,
                    std::array<int32_t, 2> slice_size_, bool additive_,
                    std::vector<float> data_, bool processed_ = false)
        : PacketBase<SliceDataPacket>(packet_desc::slice_data),
          scene_id(scene_id_), slice_id(slice_id_), slice_size(slice_size_),
          additive(additive_), data(data_), processed(processed_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | slice_id;
        buffer | slice_size;
        buffer | additive;
        buffer | data;
        buffer | processed;
    }

    int32_t scene_id;
    int32_t slice_id;
    std::array<int32_t, 2> slice_size;
    bool additive;
    std::vector<float> data;
    bool processed;
};

class PartialSliceDataPacket : public PacketBase<PartialSliceDataPacket> {
  public:
    PartialSliceDataPacket()
        : PacketBase<PartialSliceDataPacket>(packet_desc::partial_slice_data),
          scene_id(-1), slice_id(0) {}

    PartialSliceDataPacket(int32_t scene_id_, int32_t slice_id_,
                           std::array<int32_t, 2> slice_offset_,
                           std::array<int32_t, 2> slice_size_,
                           std::array<int32_t, 2> global_slice_size_,
                           bool additive_, std::vector<float> data_)
        : PacketBase<PartialSliceDataPacket>(packet_desc::partial_slice_data),
          scene_id(scene_id_), slice_id(slice_id_), slice_offset(slice_offset_),
          slice_size(slice_size_), global_slice_size(global_slice_size_),
          additive(additive_), data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | slice_id;
        buffer | slice_size;
        buffer | additive;
        buffer | data;
    }

    int32_t scene_id;
    int32_t slice_id;
    std::array<int32_t, 2> slice_offset;
    std::array<int32_t, 2> slice_size;
    std::array<int32_t, 2> global_slice_size;
    bool additive;
    std::vector<float> data;
};

class VolumeDataPacket : public PacketBase<VolumeDataPacket> {
  public:
    VolumeDataPacket()
        : PacketBase<VolumeDataPacket>(packet_desc::volume_data) {}

    VolumeDataPacket(int32_t scene_id_, std::array<int32_t, 3> volume_size_,
                     std::vector<float> data_)
        : PacketBase<VolumeDataPacket>(packet_desc::volume_data),
          scene_id(scene_id_), volume_size(volume_size_), data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | volume_size;
        buffer | data;
    }

    int32_t scene_id;
    std::array<int32_t, 3> volume_size;
    std::vector<float> data;
};

class PartialVolumeDataPacket : public PacketBase<PartialVolumeDataPacket> {
  public:
    PartialVolumeDataPacket()
        : PacketBase<PartialVolumeDataPacket>(
              packet_desc::partial_volume_data) {}

    PartialVolumeDataPacket(int32_t scene_id_,
                            std::array<int32_t, 3> volume_offset_,
                            std::array<int32_t, 3> volume_size_,
                            std::array<int32_t, 3> global_volume_size_,
                            std::vector<float> data_)
        : PacketBase<PartialVolumeDataPacket>(packet_desc::partial_volume_data),
          scene_id(scene_id_), volume_offset(volume_offset_),
          volume_size(volume_size_), global_volume_size(global_volume_size_),
          data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | volume_size;
        buffer | data;
    }

    int32_t scene_id;
    std::array<int32_t, 3> volume_offset;
    std::array<int32_t, 3> volume_size;
    std::array<int32_t, 3> global_volume_size;
    std::vector<float> data;
};

class SetSlicePacket : public PacketBase<SetSlicePacket> {
  public:
    SetSlicePacket()
        : PacketBase<SetSlicePacket>(packet_desc::set_slice), scene_id(-1),
          slice_id(0) {}

    SetSlicePacket(int32_t scene_id_, int32_t slice_id_,
                   const std::array<float, 9>& orientation_)
        : PacketBase<SetSlicePacket>(packet_desc::set_slice),
          scene_id(scene_id_), slice_id(slice_id_), orientation(orientation_) {}

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
          scene_id(-1), slice_id(0) {}

    RemoveSlicePacket(int32_t scene_id_, int32_t slice_id_)
        : PacketBase<RemoveSlicePacket>(packet_desc::remove_slice),
          scene_id(scene_id_), slice_id(slice_id_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | slice_id;
    }

    int32_t scene_id;
    int32_t slice_id;
};

class GroupRequestSlicesPacket : public PacketBase<GroupRequestSlicesPacket> {
  public:
    GroupRequestSlicesPacket()
        : PacketBase<GroupRequestSlicesPacket>(
              packet_desc::group_request_slices),
          scene_id(-1), group_size(1) {}

    GroupRequestSlicesPacket(int32_t scene_id_, int32_t group_size_)
        : PacketBase<GroupRequestSlicesPacket>(
              packet_desc::group_request_slices),
          scene_id(scene_id_), group_size(group_size_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | group_size;
    }

    int32_t scene_id;
    int32_t group_size;
};

class PostProcessPacket : public PacketBase<PostProcessPacket> {
  public:
    PostProcessPacket()
        : PacketBase<PostProcessPacket>(packet_desc::post_process_set),
          scene_id(-1), enabled(false) {}

    PostProcessPacket(int32_t scene_id_, bool enabled_)
        : PacketBase<PostProcessPacket>(packet_desc::post_process_set),
          scene_id(scene_id_), enabled(enabled_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
    }

    int32_t scene_id;
    bool enabled;
};

} // namespace tomop
