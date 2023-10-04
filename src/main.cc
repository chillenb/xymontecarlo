#include <pybind11/pybind11.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)
#include "model.h"

int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: xymontecarlo

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";
    py::class_<Model>(m, "Model")
        .def(py::init<Real, Real>())
        .def("makepasses", &Model::makepasses)
        .def("makepass", &Model::makepass);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
