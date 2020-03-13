#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <tomop/tomop.hpp>

#include <boost/hana.hpp>
using namespace hana::literals;
using namespace std::string_literals;

PYBIND11_MODULE(py_tomop, m) {
    using namespace tomop;

    m.doc() = "bindings for tomopackets";

    py::class_<tomop::Packet>(m, "packet");

    auto packets = hana::make_tuple(
        hana::make_tuple("geometry_specification_packet"s,
                         hana::type_c<GeometrySpecificationPacket>),
        hana::make_tuple("scan_settings_packet"s,
                         hana::type_c<ScanSettingsPacket>),
        hana::make_tuple("parallel_beam_geometry_packet"s,
                         hana::type_c<ParallelBeamGeometryPacket>),
        hana::make_tuple("parallel_vec_geometry_packet"s,
                         hana::type_c<ParallelVecGeometryPacket>),
        hana::make_tuple("cone_beam_geometry_packet"s,
                         hana::type_c<ConeBeamGeometryPacket>),
        hana::make_tuple("cone_vec_geometry_packet"s,
                         hana::type_c<ConeVecGeometryPacket>),
        hana::make_tuple("projection_data_packet"s,
                         hana::type_c<ProjectionDataPacket>),
        hana::make_tuple("partial_projection_data_packet"s,
                         hana::type_c<PartialProjectionDataPacket>),
        hana::make_tuple("projection_packet"s, hana::type_c<ProjectionPacket>),
        hana::make_tuple("set_part_packet"s, hana::type_c<SetPartPacket>),
        hana::make_tuple("slice_data_packet"s, hana::type_c<SliceDataPacket>),
        hana::make_tuple("partial_slice_data_packet"s,
                         hana::type_c<PartialSliceDataPacket>),
        hana::make_tuple("volume_data_packet"s, hana::type_c<VolumeDataPacket>),
        hana::make_tuple("partial_volume_data_packet"s,
                         hana::type_c<PartialVolumeDataPacket>),
        hana::make_tuple("set_slice_packet"s, hana::type_c<SetSlicePacket>),
        hana::make_tuple("remove_slice_packet"s,
                         hana::type_c<RemoveSlicePacket>),
        hana::make_tuple("group_request_slices_packet"s,
                         hana::type_c<GroupRequestSlicesPacket>),
        hana::make_tuple("make_scene_packet"s, hana::type_c<MakeScenePacket>),
        hana::make_tuple("kill_scene_packet"s, hana::type_c<KillScenePacket>),
        hana::make_tuple("parameter_bool_packet"s,
                         hana::type_c<ParameterBoolPacket>),
        hana::make_tuple("parameter_float_packet"s,
                         hana::type_c<ParameterFloatPacket>),
        hana::make_tuple("parameter_enum_packet"s,
                         hana::type_c<ParameterEnumPacket>),
        hana::make_tuple("tracker_packet"s, hana::type_c<TrackerPacket>),
        hana::make_tuple("benchmark_packet"s, hana::type_c<BenchmarkPacket>));

    hana::for_each(packets, [&](auto x) {
        // 1) get C++ type
        using P = typename decltype(+(x[1_c]))::type;

        // 2) get constructor args
        auto types = hana::transform(hana::members(P{}), [](auto member) {
            return hana::type_c<decltype(member)>;
        });
        using Init = typename decltype(hana::unpack(
            types, hana::template_<py::detail::initimpl::constructor>))::type;

        // We also want to add named arguments to the constructor
        auto names = hana::transform(
            hana::keys(P{}), [](auto key) { return py::arg(key.c_str()); });
        auto indirection = [&](auto... args) {
            auto pack = py::class_<P, tomop::Packet>(m, x[0_c].c_str())
                            .def(Init(), args...);
            return pack;
        };

        // 3) register class with Python
        auto pack = hana::unpack(names, indirection);
        hana::fold(hana::accessors<P>(), std::ref(pack),
                   [](py::class_<P, tomop::Packet>& c,
                      auto ka) -> py::class_<P, tomop::Packet>& {
                       return c.def(hana::first(ka).c_str(), [&ka](P& p) {
                           return hana::second(ka)(p);
                       });
                   });
    });

    py::class_<tomop::server>(m, "server")
        .def(py::init<std::string>())
        .def(py::init<std::string, std::string, std::string>())
        .def(py::init<int32_t>())
        .def(py::init<int32_t, int32_t, std::string, std::string>())
        .def("scene_id", &tomop::server::scene_id)
        .def("set_callback", &tomop::server::set_slice_callback)
        .def("set_projection_callback", &tomop::server::set_projection_callback)
        .def("serve", &tomop::server::serve,
             py::call_guard<py::gil_scoped_release>())
        .def("listen", &tomop::server::listen,
             py::call_guard<py::gil_scoped_release>())
        .def("send", &tomop::server::send);

    py::class_<tomop::publisher>(m, "publisher")
        .def(py::init<std::string, int32_t>())
        .def(py::init<std::string, int32_t, int32_t>())
        .def("send", &tomop::publisher::send);
}
