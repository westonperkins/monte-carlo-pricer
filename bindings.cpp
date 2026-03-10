#include <pybind11/pybind11.h>
#include <random>
#include <pybind11/stl.h>
#include "mc_pricer.h"

namespace py = pybind11;

// -----------------------------
// Wrapper Functions
// -----------------------------

double call_price_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N)
{
    std::mt19937 rng(42);
    return monte_carlo_call(S0, K, r, sigma, T, N, rng);
}

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

double delta_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h = 1e-4)
{
    std::mt19937 rng(42);
    return monte_carlo_delta(S0, K, r, sigma, T, N, h, rng);
}

MCResult call_price_full_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N)
{
    std::mt19937 rng(42);
    return monte_carlo_call_with_greeks(
        S0, K, r, sigma, T, N, rng);
}

MCResult call_price_full_antithetic_py(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N)
{
    std::mt19937 rng(42);
    return monte_carlo_call_antithetic_with_greeks(
        S0, K, r, sigma, T, N, rng);
}

// -----------------------------
// Python Module
// -----------------------------

PYBIND11_MODULE(mc_pricer_py, m)
{
    m.doc() = "Monte Carlo option pricer (C++ backend)";

    py::class_<MCResult>(m, "MCResult")
        .def_readonly("price", &MCResult::price)
        .def_readonly("delta", &MCResult::delta)
        .def_readonly("std_error", &MCResult::std_error)
        .def_readonly("ci_lower", &MCResult::ci_lower)
        .def_readonly("ci_upper", &MCResult::ci_upper);

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

    m.def("call_price_full", &call_price_full_py,
          py::arg("S0"), py::arg("K"), py::arg("r"),
          py::arg("sigma"), py::arg("T"), py::arg("N"));

    m.def("call_price_full_antithetic",
          &call_price_full_antithetic_py,
          py::arg("S0"), py::arg("K"), py::arg("r"),
          py::arg("sigma"), py::arg("T"), py::arg("N"));

    // -----------------------------
    // Implied Volatility
    // -----------------------------

    m.def("implied_volatility_call",
          &implied_volatility_call,
          py::arg("market_price"),
          py::arg("S0"),
          py::arg("K"),
          py::arg("r"),
          py::arg("T"),
          py::arg("initial_guess") = 0.2,
          py::arg("max_iterations") = 100,
          py::arg("tolerance") = 1e-8);

    // -----------------------------
    // Trade Evaluation Binding
    // -----------------------------

    py::class_<MCTradeStats>(m, "MCTradeStats")
        .def_readonly("expected_pnl", &MCTradeStats::expected_pnl)
        .def_readonly("prob_profit", &MCTradeStats::prob_profit)
        .def_readonly("prob_itm", &MCTradeStats::prob_itm)
        .def_readonly("prob_breakeven", &MCTradeStats::prob_breakeven)
        .def_readonly("pnl_paths", &MCTradeStats::pnl_paths);

    m.def("trade_stats", [](double S0, double K, double r, double sigma, double T, double mu, double premium, int N)
          {
          std::mt19937 rng(42);
          return monte_carlo_trade_stats(
              S0, K, r, sigma, T,
              mu, premium, N, rng); }, py::arg("S0"), py::arg("K"), py::arg("r"), py::arg("sigma"), py::arg("T"), py::arg("mu"), py::arg("premium"), py::arg("N"));
}