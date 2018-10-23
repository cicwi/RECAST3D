#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <slicerecon/slicerecon.hpp>

using namespace slicerecon;

PYBIND11_MODULE(py_slicerecon, m) {
    m.doc() = "bindings for slicerecon";

    py::class_<plugin>(m, "plugin")
        .def(py::init<std::string, std::string>(), "Initialize the plugin",
             py::arg("hostname_in") = "tcp://*:5650",
             py::arg("hostname_out") = "tcp://localhost:5555")
        .def("set_slice_callback", &plugin::set_slice_callback)
        .def("serve", &plugin::serve, py::call_guard<py::gil_scoped_release>())
      .def("listen", &plugin::listen, py::call_guard<py::gil_scoped_release>());
}
