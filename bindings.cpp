#include <pybind11/pybind11.h>
#include "mc_pricer.h"

namespace py = pybind11;

PYBIND11_MODULE(mc_pricer_py, m) {
    m.doc() = "Monte Carlo option pricer (C++ backend)";

    m.def(
        "call_price",
        &monte_carlo_call,
        py::arg("S0"),
        py::arg("K"),
        py::arg("r"),
        py::arg("sigma"),
        py::arg("T"),
        py::arg("N"),
        "Monte Carlo European call option price"
    );
}
