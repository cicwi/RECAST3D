#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;


#include <slicerecon/slicerecon.hpp>

using namespace slicerecon;

PYBIND11_MODULE(py_slicerecon, m) {
    m.doc() = "bindings for slicerecon";

    py::class_<slicerecon::plugin>(m, "plugin");
}
