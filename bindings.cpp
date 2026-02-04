#include <pybind11/pybind11.h>
#include <random>
#include "mc_pricer.h"

// python bindings for the monte carlo pricing engine
// exposes the C++ monte carlo pricer to python using pybind11
// goal: keep all heavy numerical computation in C++
// provide clean python python friendly API
// allow fast experimentation, plotting, and analysis from python
// python users do not interact w RNG directly - manages randomness on their behalf
namespace py = pybind11;

// python safe wrapper functions
// 1. create local random number generator
// 2. calling the underlying C++ Monte carlo functions
// 3. returning simple numeric results to python

// standard monte carlo call option price
double call_price_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N)
{
    // deterministic RNG seed for reproducibility
    std::mt19937 rng(42); // deterministic for now
    return monte_carlo_call(S0, K, r, sigma, T, N, rng);
}

// antithetic monte carlo call option price
// uses variance reduction by pairing randomn paths with their negatives to reduce noise
double call_price_antithetic_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N)
{
    std::mt19937 rng(42);
    return monte_carlo_call_antithetic(S0, K, r, sigma, T, N, rng);
}

// finite difference delta (diagnostic)
// computes option sensitivity to the stock price using finite differences
// this method is slower and noisier than the pathwise delta, but userful for validation
double delta_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h = 1e-4 // small perturbation size
)
{
    std::mt19937 rng(42);
    return monte_carlo_delta(S0, K, r, sigma, T, N, h, rng);
}

// python module defintion
// this block defines the python module name and the functions that become avaliable in python
PYBIND11_MODULE(mc_pricer_py, m)
{
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
