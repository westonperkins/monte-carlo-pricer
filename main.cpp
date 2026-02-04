#include <iostream>
#include <random>
#include "mc_pricer.h"
#include "payoffs.h"

int main()
{
    // basic option and market parameters
    double S0 = 100.0;  // current stock price
    double K = 105.0;   // option strike price
    double r = 0.05;    // risk free interest rate (annual)
    double sigma = 0.2; // sigma - volatility (annual)
    double T = 1.0;     // time to maturity (years)
    int N = 1'000'000;  // number of simulations

    // independent random number generations - seed 42
    std::mt19937 rng_price(42);
    std::mt19937 rng_antithetic(42);
    std::mt19937 rng_fd_delta(42);
    std::mt19937 rng_greeks(42);
    std::mt19937 rng_ant_greeks(42);

    // standard monte carlo pricing - estimates options price by simulating possible futures and averaging payoff at maturity
    double price_std = monte_carlo_call(
        S0, K, r, sigma, T, N, rng_price);

    // antithetic monte carlo pricing - paired simulations (Z and -Z) to reduce randomness and improve convergence while maintaining compute time
    double price_ant = monte_carlo_call_antithetic(
        S0, K, r, sigma, T, N, rng_antithetic);

    // finite difference delta (diagnostic) - approximates sensitivity of the option price
    double delta_fd = monte_carlo_delta(
        S0, K, r, sigma, T, N, 0.01, rng_fd_delta);

    // single pass monte carlo price and delta - computes option price and delta in one simulation using pathwise differentiation
    MCResult res = monte_carlo_call_with_greeks(
        S0, K, r, sigma, T, N, rng_greeks);

    // antithetic single pass price and delta - combines variance reduction w single pass greek estimation
    MCResult res_ant = monte_carlo_call_antithetic_with_greeks(
        S0, K, r, sigma, T, N, rng_ant_greeks);

    // generic payoff based pricing - demonstrates that the engine is payoff agnostic and price different contacts w/o modifying the core logic

    CallPayoff call(K);
    PutPayoff put(K);

    std::mt19937 rng_call(42);
    std::mt19937 rng_put(42);

    double call_price = monte_carlo_price(
        S0, r, sigma, T, N, call, rng_call);

    double put_price = monte_carlo_price(
        S0, r, sigma, T, N, put, rng_put);

    std::cout << "Generic Call Price : " << call_price << "\n";
    std::cout << "Generic Put Price  : " << put_price << "\n";

    // output results - prices and greeks from different monte carlo engines are printed to compare accuracy, convergence, and variance reduction techniques
    std::cout << "Antithetic MC Price : " << res_ant.price << '\n';
    std::cout << "Antithetic MC Delta : " << res_ant.delta << '\n';

    std::cout << "MC Price (single-pass) : " << res.price << '\n';
    std::cout << "MC Delta (single-pass) : " << res.delta << '\n';

    std::cout << "Standard MC            : " << price_std << '\n';
    std::cout << "Antithetic MC          : " << price_ant << '\n';
    std::cout << "FD Delta (diagnostic)  : " << delta_fd << '\n';

    return 0;
}
