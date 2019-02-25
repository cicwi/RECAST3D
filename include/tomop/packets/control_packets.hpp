#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

struct ParameterBoolPacket : public PacketBase<ParameterBoolPacket> {
    static const auto desc = packet_desc::parameter_bool;
    ParameterBoolPacket() = default;
    ParameterBoolPacket(int32_t a, std::string b, bool c)
        : scene_id(a), parameter_name(b), value(c) {}
    BOOST_HANA_DEFINE_STRUCT(ParameterBoolPacket, (int32_t, scene_id),
                             (std::string, parameter_name), (bool, value));
};

struct ParameterFloatPacket : public PacketBase<ParameterFloatPacket> {
    static const auto desc = packet_desc::parameter_float;
    ParameterFloatPacket() = default;
    ParameterFloatPacket(int32_t a, std::string b, float c)
        : scene_id(a), parameter_name(b), value(c) {}
    BOOST_HANA_DEFINE_STRUCT(ParameterFloatPacket, (int32_t, scene_id),
                             (std::string, parameter_name), (float, value));
};

struct ParameterEnumPacket : public PacketBase<ParameterEnumPacket> {
    static const auto desc = packet_desc::parameter_enum;
    ParameterEnumPacket() = default;
    ParameterEnumPacket(int32_t a, std::string b, std::vector<std::string> c)
        : scene_id(a), parameter_name(b), values(c) {}
    BOOST_HANA_DEFINE_STRUCT(ParameterEnumPacket, (int32_t, scene_id),
                             (std::string, parameter_name),
                             (std::vector<std::string>, values));
};

struct TrackerPacket : public PacketBase<TrackerPacket> {
    static const auto desc = packet_desc::tracker;
    TrackerPacket() = default;
    TrackerPacket(int32_t a, std::string b, float c)
        : scene_id(a), parameter_name(b), value(c) {}
    BOOST_HANA_DEFINE_STRUCT(TrackerPacket, (int32_t, scene_id),
                             (std::string, parameter_name), (float, value));
};

struct BenchmarkPacket : public PacketBase<BenchmarkPacket> {
    static const auto desc = packet_desc::benchmark;
    BenchmarkPacket() = default;
    BenchmarkPacket(int32_t a, std::string b, float c)
        : scene_id(a), parameter_name(b), value(c) {}
    BOOST_HANA_DEFINE_STRUCT(BenchmarkPacket, (int32_t, scene_id),
                             (std::string, parameter_name), (float, value));
};

} // namespace tomop
