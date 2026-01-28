#include <iostream>
#include <random>
#include "mc_pricer.h"

int main()
{
    double S0 = 100.0;
    double K = 105.0;
    double r = 0.05;
    double sigma = 0.2;
    double T = 1.0;
    int N = 1'000'000;

    // --------------------------------------------------
    // Independent RNGs (same seed, separate streams)
    // --------------------------------------------------
    std::mt19937 rng_price(42);
    std::mt19937 rng_antithetic(42);
    std::mt19937 rng_fd_delta(42);
    std::mt19937 rng_greeks(42);
    std::mt19937 rng_ant_greeks(42);

    // --------------------------------------------------
    // Standard Monte Carlo
    // --------------------------------------------------
    double price_std = monte_carlo_call(
        S0, K, r, sigma, T, N, rng_price);

    // --------------------------------------------------
    // Antithetic Monte Carlo
    // --------------------------------------------------
    double price_ant = monte_carlo_call_antithetic(
        S0, K, r, sigma, T, N, rng_antithetic);

    // --------------------------------------------------
    // Finite-difference Delta (diagnostic only)
    // --------------------------------------------------
    double delta_fd = monte_carlo_delta(
        S0, K, r, sigma, T, N, 0.01, rng_fd_delta);

    // --------------------------------------------------
    // Single-pass price + Greeks
    // --------------------------------------------------
    MCResult res = monte_carlo_call_with_greeks(
        S0, K, r, sigma, T, N, rng_greeks);

    // --------------------------------------------------
    // Antithetic single-pass price + Greeks (best engine)
    // --------------------------------------------------
    MCResult res_ant = monte_carlo_call_antithetic_with_greeks(
        S0, K, r, sigma, T, N, rng_ant_greeks);

    // --------------------------------------------------
    // Output
    // --------------------------------------------------
    std::cout << "Antithetic MC Price : " << res_ant.price << '\n';
    std::cout << "Antithetic MC Delta : " << res_ant.delta << '\n';

    std::cout << "MC Price (single-pass) : " << res.price << '\n';
    std::cout << "MC Delta (single-pass) : " << res.delta << '\n';

    std::cout << "Standard MC            : " << price_std << '\n';
    std::cout << "Antithetic MC          : " << price_ant << '\n';
    std::cout << "FD Delta (diagnostic)  : " << delta_fd << '\n';

    return 0;
}
