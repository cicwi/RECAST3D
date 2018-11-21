#pragma once

namespace tomop {

enum class packet_desc : int {
    // SCENE MANAGEMENT
    make_scene = 0x101,
    kill_scene = 0x102,

    // RECONSTRUCTION
    slice_data = 0x201,
    partial_slice_data = 0x202,
    volume_data = 0x203,
    partial_volume_data = 0x204,
    set_slice = 0x205,
    remove_slice = 0x206,
    group_request_slices = 0x207,
    register_parameter = 0x208,
    parameter_changed = 0x209,

    // GEOMETRY
    geometry_specification = 0x301,
    scan_settings = 0x302,
    parallel_beam_geometry = 0x303,
    parallel_vec_geometry = 0x304,
    cone_beam_geometry = 0x305,
    cone_vec_geometry = 0x306,
    projection_data = 0x307,
    partial_projection_data = 0x308,
    projection = 0x309,

    // PARTITIONING
    set_part = 0x401,
};

} // namespace tomop
