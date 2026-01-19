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
        "Standard Monte Carlo European call option price"
    );

    m.def(
        "call_price_antithetic",
        &monte_carlo_call_antithetic,
        py::arg("S0"),
        py::arg("K"),
        py::arg("r"),
        py::arg("sigma"),
        py::arg("T"),
        py::arg("N"),
        "Antithetic Monte Carlo European call option price"
    );

    m.def(
        "delta",
        &monte_carlo_delta,
        py::arg("S0"),
        py::arg("K"),
        py::arg("r"),
        py::arg("sigma"),
        py::arg("T"),
        py::arg("N"),
        py::arg("h") = 1e-4,
        "Monte Carlo delta via central finite differences"
    );
}
