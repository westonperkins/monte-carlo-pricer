#include "mc_pricer.h"
#include <random>
#include <cmath>
#include <algorithm>
#include "payoff.h"

namespace
{
    // simulate stock price at option maturity
    // uses standard geometric brownian motion model to evolve the stock price from today (S0) to maturity (T) given a random shock Z
    // represents market model
    inline double simulate_terminal_price(
        double S0,
        double r,
        double sigma,
        double T,
        double Z)
    {
        return S0 * std::exp(
                        (r - 0.5 * sigma * sigma) * T +
                        sigma * std::sqrt(T) * Z);
    }

    // generic monte carlo engine
    // core simulation loop:
    // 1. draws random samples
    // 2. evaluates a payoff function
    // 3. accumulates price and greek contributions
    // 4. discounts the result to present value
    // generic and doesn't know anything about option types or payoff formulas
    template <typename PayoffFunc, typename DeltaFunc>
    MCResult monte_carlo_engine(
        int N,                   // number of simulations
        double r,                // risk free rate
        double T,                // time to maturity
        std::mt19937 &rng,       // random number generator
        PayoffFunc payoff,       // payoff logic
        DeltaFunc delta_contrib) // delta contribution logic
    {
        std::normal_distribution<> dist(0.0, 1.0);

        double payoff_sum = 0.0;
        double delta_sum = 0.0;

        for (int i = 0; i < N; ++i)
        {
            double Z = dist(rng);
            payoff_sum += payoff(Z);
            delta_sum += delta_contrib(Z);
        }

        double discount = std::exp(-r * T);

        MCResult result;
        result.price = discount * (payoff_sum / N);
        result.delta = discount * (delta_sum / N);
        return result;
    }

} // anonymous namespace

// standard monte carlo call option pricing
// computes the price of a european call option using monte carlo simulation without variance reduction and without greeks
double monte_carlo_call(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    auto payoff = [&](double Z)
    {
        double ST = simulate_terminal_price(S0, r, sigma, T, Z);
        return std::max(ST - K, 0.0);
    };

    auto zero_delta = [&](double)
    {
        return 0.0;
    };

    MCResult res = monte_carlo_engine(
        N, r, T, rng, payoff, zero_delta);

    return res.price;
}

// finite difference delta (diagnostic)
// approximates delta by re pricing the option at slightly higher and lower stock prices
// this method is computationally expensive and noisy, and is included only for comparison purposes
double monte_carlo_delta(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h,
    std::mt19937 &rng)
{
    double price_up =
        monte_carlo_call(S0 + h, K, r, sigma, T, N, rng);

    double price_down =
        monte_carlo_call(S0 - h, K, r, sigma, T, N, rng);

    return (price_up - price_down) / (2.0 * h);
}

// antithetic monte carlo call pricing
// uses paired random samples (Z and -Z) to reduce simulaiton noise and imporve convergence while preserving computational cost
double monte_carlo_call_antithetic(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    int half_N = N / 2;

    auto payoff = [&](double Z)
    {
        double ST_pos = simulate_terminal_price(S0, r, sigma, T, Z);
        double ST_neg = simulate_terminal_price(S0, r, sigma, T, -Z);

        return 0.5 * (std::max(ST_pos - K, 0.0) +
                      std::max(ST_neg - K, 0.0));
    };

    auto zero_delta = [&](double)
    {
        return 0.0;
    };

    MCResult res = monte_carlo_engine(
        half_N, r, T, rng, payoff, zero_delta);

    return res.price;
}

// single pass monte carlo price and delta
// computes both the option price and delta in a single simulation pass using pathwise differentiation
// more efficient and accurate than finite difference mehtods
MCResult monte_carlo_call_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    auto payoff = [&](double Z)
    {
        double ST = simulate_terminal_price(S0, r, sigma, T, Z);
        return std::max(ST - K, 0.0);
    };

    auto delta_contrib = [&](double Z)
    {
        double ST = simulate_terminal_price(S0, r, sigma, T, Z);
        return (ST > K) ? (ST / S0) : 0.0;
    };

    return monte_carlo_engine(
        N, r, T, rng, payoff, delta_contrib);
}

// antithetic single pass monte carlo price and delta
// combines variance reduction with single pass greeks estimation - most accurate and efficient
MCResult monte_carlo_call_antithetic_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    int half_N = N / 2;

    auto payoff = [&](double Z)
    {
        double ST_pos = simulate_terminal_price(S0, r, sigma, T, Z);
        double ST_neg = simulate_terminal_price(S0, r, sigma, T, -Z);

        return 0.5 * (std::max(ST_pos - K, 0.0) +
                      std::max(ST_neg - K, 0.0));
    };

    auto delta_contrib = [&](double Z)
    {
        double ST_pos = simulate_terminal_price(S0, r, sigma, T, Z);
        double ST_neg = simulate_terminal_price(S0, r, sigma, T, -Z);

        double d = 0.0;
        if (ST_pos > K)
            d += 0.5 * (ST_pos / S0);
        if (ST_neg > K)
            d += 0.5 * (ST_neg / S0);
        return d;
    };

    return monte_carlo_engine(
        half_N, r, T, rng, payoff, delta_contrib);
}

// generic payoff based monte carlo pricing
// prices any european style derivative using a user supplied payoff definition - simulation engine is independent of contract type 
double monte_carlo_price(
    double S0,
    double r,
    double sigma,
    double T,
    int N,
    const Payoff &payoff,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);

    double payoff_sum = 0.0;
    double drift = (r - 0.5 * sigma * sigma) * T;
    double diffusion = sigma * std::sqrt(T);

    for (int i = 0; i < N; ++i)
    {
        double Z = dist(rng);
        double ST = S0 * std::exp(drift + diffusion * Z);
        payoff_sum += payoff(ST);
    }

    return std::exp(-r * T) * (payoff_sum / N);
}