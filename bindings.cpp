#include <pybind11/pybind11.h>
#include <random>
#include "mc_pricer.h"

namespace py = pybind11;

// ---------- Python-safe wrappers ----------

double call_price_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N
) {
    std::mt19937 rng(42);  // deterministic for now
    return monte_carlo_call(S0, K, r, sigma, T, N, rng);
}

double call_price_antithetic_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N
) {
    std::mt19937 rng(42);
    return monte_carlo_call_antithetic(S0, K, r, sigma, T, N, rng);
}

double delta_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h = 1e-4
) {
    std::mt19937 rng(42);
    return monte_carlo_delta(S0, K, r, sigma, T, N, h, rng);
}

// ---------- Python module ----------

PYBIND11_MODULE(mc_pricer_py, m) {
    m.doc() = "Monte Carlo option pricer (C++ backend)";

    m.def("call_price", &call_price_py,
          py::arg("S0"), py::arg("K"), py::arg("r"),
          py::arg("sigma"), py::arg("T"), py::arg("N"));

    m.def("call_price_antithetic", &call_price_antithetic_py,
          py::arg("S0"), py::arg("K"), py::arg("r"),
          py::arg("sigma"), py::arg("T"), py::arg("N"));

    m.def("delta", &delta_py,
          py::arg("S0"), py::arg("K"), py::arg("r"),
          py::arg("sigma"), py::arg("T"),
          py::arg("N"), py::arg("h") = 1e-4);
}
