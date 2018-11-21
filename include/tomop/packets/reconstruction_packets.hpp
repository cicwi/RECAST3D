#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

struct SliceDataPacket : public PacketBase<SliceDataPacket> {
    static const auto desc = packet_desc::slice_data;
    SliceDataPacket() = default;
    SliceDataPacket(int32_t a, int32_t b, std::array<int32_t, 2> c,
                    std::vector<float> d, bool e)
        : scene_id(a), slice_id(b), slice_size(c), data(d), additive(e) {}
    BOOST_HANA_DEFINE_STRUCT(SliceDataPacket, (int32_t, scene_id),
                             (int32_t, slice_id),
                             (std::array<int32_t, 2>, slice_size),
                             (std::vector<float>, data), (bool, additive));
};

struct PartialSliceDataPacket : public PacketBase<PartialSliceDataPacket> {
    static const auto desc = packet_desc::partial_slice_data;
    PartialSliceDataPacket() = default;
    PartialSliceDataPacket(int32_t a, int32_t b, std::array<int32_t, 2> c,
                           std::array<int32_t, 2> d, std::array<int32_t, 2> e,
                           bool f, std::vector<float> g)
        : scene_id(a), slice_id(b), slice_offset(c), slice_size(d),
          global_slice_size(e), additive(f), data(g) {}
    BOOST_HANA_DEFINE_STRUCT(PartialSliceDataPacket, (int32_t, scene_id),
                             (int32_t, slice_id),
                             (std::array<int32_t, 2>, slice_offset),
                             (std::array<int32_t, 2>, slice_size),
                             (std::array<int32_t, 2>, global_slice_size),
                             (bool, additive), (std::vector<float>, data));
};

struct VolumeDataPacket : public PacketBase<VolumeDataPacket> {
    static const auto desc = packet_desc::volume_data;
    VolumeDataPacket() = default;
    VolumeDataPacket(int32_t a, std::array<int32_t, 3> b, std::vector<float> c)
        : scene_id(a), volume_size(b), data(c) {}
    BOOST_HANA_DEFINE_STRUCT(VolumeDataPacket, (int32_t, scene_id),
                             (std::array<int32_t, 3>, volume_size),
                             (std::vector<float>, data));
};

struct PartialVolumeDataPacket : public PacketBase<PartialVolumeDataPacket> {
    static const auto desc = packet_desc::partial_volume_data;
    PartialVolumeDataPacket() = default;
    PartialVolumeDataPacket(int32_t a, std::array<int32_t, 3> b,
                            std::array<int32_t, 3> c, std::array<int32_t, 3> d,
                            std::vector<float> e)
        : scene_id(a), volume_offset(b), volume_size(c), global_volume_size(d),
          data(e) {}
    BOOST_HANA_DEFINE_STRUCT(PartialVolumeDataPacket, (int32_t, scene_id),
                             (std::array<int32_t, 3>, volume_offset),
                             (std::array<int32_t, 3>, volume_size),
                             (std::array<int32_t, 3>, global_volume_size),
                             (std::vector<float>, data));
};

struct SetSlicePacket : public PacketBase<SetSlicePacket> {
    static const auto desc = packet_desc::set_slice;
    SetSlicePacket() = default;
    SetSlicePacket(int32_t a, int32_t b, std::array<float, 9> c)
        : scene_id(a), slice_id(b), orientation(c) {}
    BOOST_HANA_DEFINE_STRUCT(SetSlicePacket, (int32_t, scene_id),
                             (int32_t, slice_id),
                             (std::array<float, 9>, orientation));
};

struct RemoveSlicePacket : public PacketBase<RemoveSlicePacket> {
    static const auto desc = packet_desc::remove_slice;
    RemoveSlicePacket() = default;
    RemoveSlicePacket(int32_t a, int32_t b) : scene_id(a), slice_id(b) {}
    BOOST_HANA_DEFINE_STRUCT(RemoveSlicePacket, (int32_t, scene_id),
                             (int32_t, slice_id));
};

struct GroupRequestSlicesPacket : public PacketBase<GroupRequestSlicesPacket> {
    static const auto desc = packet_desc::group_request_slices;
    GroupRequestSlicesPacket() = default;
    GroupRequestSlicesPacket(int32_t a, int32_t b)
        : scene_id(a), group_size(b) {}
    BOOST_HANA_DEFINE_STRUCT(GroupRequestSlicesPacket, (int32_t, scene_id),
                             (int32_t, group_size));
};

struct RegisterParameterPacket : public PacketBase<RegisterParameterPacket> {
    static const auto desc = packet_desc::register_parameter;
    RegisterParameterPacket() = default;
    RegisterParameterPacket(int32_t a, std::string b, float c)
        : scene_id(a), parameter_name(b), initial_value(c) {}
    BOOST_HANA_DEFINE_STRUCT(RegisterParameterPacket, (int32_t, scene_id),
                             (std::string, parameter_name),
                             (float, initial_value));
};

struct ParameterChangedPacket : public PacketBase<ParameterChangedPacket> {
    static const auto desc = packet_desc::parameter_changed;
    ParameterChangedPacket() = default;
    ParameterChangedPacket(int32_t a, std::string b, float c)
        : scene_id(a), parameter_name(b), value(c) {}
    BOOST_HANA_DEFINE_STRUCT(ParameterChangedPacket, (int32_t, scene_id),
                             (std::string, parameter_name),
                             (float, value));
};

} // namespace tomop
