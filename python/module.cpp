#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <tomop/tomop.hpp>

PYBIND11_MODULE(py_tomop, m) {
    m.doc() = "bindings for tomopackets";

    py::class_<tomop::Packet>(m, "packet");

    py::class_<tomop::VolumeDataPacket, tomop::Packet>(m, "volume_data_packet")
        .def(py::init<int32_t, std::vector<int32_t>, std::vector<float>>());

    py::class_<tomop::SliceDataPacket, tomop::Packet>(m, "slice_data_packet")
        .def(py::init<int32_t, int32_t, std::vector<int32_t>, bool,
                      std::vector<float>>());

    py::class_<tomop::GeometrySpecificationPacket, tomop::Packet>(
        m, "geometry_specification_packet")
        .def(py::init<int32_t, bool, int32_t>());

    py::class_<tomop::ProjectionDataPacket, tomop::Packet>(
        m, "projection_data_packet")
        .def(py::init<int32_t, int32_t, std::array<float, 3>,
                      std::array<float, 9>, std::array<int32_t, 2>,
                      std::vector<float>>());

    py::class_<tomop::server>(m, "server")
        .def(py::init<std::string>())
        .def(py::init<std::string, std::string, std::string>())
        .def(py::init<int32_t>())
        .def(py::init<int32_t, int32_t, std::string, std::string>())
        .def("scene_id", &tomop::server::scene_id)
        .def("set_callback", &tomop::server::set_slice_callback)
        .def("serve", &tomop::server::serve)
        .def("listen", &tomop::server::listen)
        .def("send", &tomop::server::send);
}
