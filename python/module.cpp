#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <tomop/tomop.hpp>

PYBIND11_PLUGIN(py_tomop) {
    py::module m("py_tomop", "bindings for tomopackets");

    py::class_<tomop::Packet>(m, "packet");

    py::class_<tomop::VolumeDataPacket, tomop::Packet>(m, "volume_data_packet")
        .def(py::init<int32_t, std::vector<int32_t>, std::vector<uint8_t>>());

    py::class_<tomop::server>(m, "server")
        .def(py::init<std::string>())
        .def(py::init<std::string, std::string, std::string>())
        .def("scene_id", &tomop::server::scene_id)
        .def("set_callback", &tomop::server::set_slice_callback)
        .def("serve", &tomop::server::serve)
        .def("send", &tomop::server::send);

    return m.ptr();
}
