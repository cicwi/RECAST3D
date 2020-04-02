#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <slicerecon/slicerecon.hpp>

using namespace slicerecon;

void push_projection(reconstructor* recon, int32_t proj_idx, py::array_t<float, py::array::c_style | py::array::forcecast> data)
{
  py::buffer_info info = data.request();
  std::array<int32_t, 2> shape = std::array<int32_t, 2> { info.shape[0], info.shape[1] };
  recon->push_projection(proj_kind::standard, proj_idx, shape, (char*)info.ptr);
}

PYBIND11_MODULE(py_slicerecon, m)
{
  m.doc() = "bindings for slicerecon";

  py::class_<plugin>(m, "plugin")
      .def(py::init<std::string, std::string>(), "Initialize the plugin",
          py::arg("hostname_in") = "tcp://*:5650",
          py::arg("hostname_out") = "tcp://localhost:5555")
      .def("set_slice_callback", &plugin::set_slice_callback)
      .def("serve", &plugin::serve, py::call_guard<py::gil_scoped_release>())
      .def("listen", &plugin::listen, py::call_guard<py::gil_scoped_release>());

  py::enum_<mode>(m, "Mode", "The reconstruction mode")
      .value("Continuous", mode::continuous)
      .value("Alternating", mode::alternating);

  slicerecon::paganin_settings paganin_default;
  py::class_<paganin_settings>(m, "paganin_settings")
      .def(py::init())
      .def(py::init<float, float, float, float, float>(), "Initialize Paganin settings",
          py::arg("pixel_size") = paganin_default.pixel_size,
          py::arg("lambda") = paganin_default.lambda,
          py::arg("delta") = paganin_default.delta,
          py::arg("beta") = paganin_default.beta,
          py::arg("distance") = paganin_default.distance);

  settings settings_default;
  py::class_<settings>(m, "settings")
      .def(py::init())
      .def(py::init<int32_t,
               int32_t,
               int32_t,
               int32_t,
               int32_t,
               int32_t,
               mode,
               bool,
               bool,
               bool,
               paganin_settings,
               bool,
               std::string>(),
          "Initialize settings",
          py::arg("slice_size") = settings_default.slice_size,
          py::arg("preview_size") = settings_default.preview_size,
          py::arg("group_size") = settings_default.group_size,
          py::arg("filter_cores") = settings_default.filter_cores,
          py::arg("darks") = settings_default.darks,
          py::arg("flats") = settings_default.flats,
          py::arg("reconstruction_mode") = settings_default.reconstruction_mode,
          py::arg("already_linear") = settings_default.already_linear,
          py::arg("retrieve_phase") = settings_default.retrieve_phase,
          py::arg("tilt_axis") = settings_default.tilt_axis,
          py::arg("paganin") = settings_default.paganin,
          py::arg("gaussian_pass") = settings_default.gaussian_pass,
          py::arg("filter") = settings_default.filter);

  py::class_<reconstructor>(m, "reconstructor")
      .def(py::init<settings>(), "Construct a slice reconstructor", py::arg("parameters"))
      .def("initialize", &reconstructor::initialize)
      .def("reconstruct_slice", &reconstructor::reconstruct_slice)
      .def("preview", &reconstructor::preview_data)
      .def("set_filter", &reconstructor::set_filter);

  acquisition::geometry geometry_default;
  py::class_<acquisition::geometry>(m, "acquisition_geometry")
      .def(py::init())
      .def(py::init<int32_t,
               int32_t,
               int32_t,
               std::vector<float>,
               bool,
               bool,
               std::array<float, 2>,
               std::array<float, 3>,
               std::array<float, 3>,
               float,
               float>(),
          "Construct an acquisition geometry",
          py::arg("rows") = geometry_default.rows,
          py::arg("cols") = geometry_default.cols,
          py::arg("proj_count") = geometry_default.proj_count,
          py::arg("angles") = geometry_default.angles,
          py::arg("parallel") = geometry_default.parallel,
          py::arg("vec_geometry") = geometry_default.vec_geometry,
          py::arg("detector_size") = geometry_default.detector_size,
          py::arg("volume_min_point") = geometry_default.volume_min_point,
          py::arg("volume_max_point") = geometry_default.volume_max_point,
          py::arg("source_origin") = geometry_default.source_origin,
          py::arg("origin_det") = geometry_default.origin_det);

  py::class_<projection>(m, "projection");

  m.def("push_projection", &push_projection);

  py::enum_<proj_kind>(m, "ProjKind", "A projection kind")
      .value("Dark", proj_kind::dark)
      .value("Light", proj_kind::light)
      .value("Standard", proj_kind::standard);
}
