#include "mc_pricer.h"
#include <random>
#include <cmath>
#include <algorithm>

namespace
{

    // Shared path evolution logic
    inline double simulate_terminal_price(
        double S0,
        double r,
        double sigma,
        double T,
        double Z)
    {
        return S0 * std::exp(
                        (r - 0.5 * sigma * sigma) * T + sigma * std::sqrt(T) * Z);
    }

} // anonymous namespace

double monte_carlo_call(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);
    double payoff_sum = 0.0;

    for (int i = 0; i < N; ++i)
    {
        double Z = dist(rng);
        double ST = simulate_terminal_price(S0, r, sigma, T, Z);
        payoff_sum += std::max(ST - K, 0.0);
    }

    return std::exp(-r * T) * (payoff_sum / N);
}

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
    double price_up = monte_carlo_call(S0 + h, K, r, sigma, T, N, rng);
    double price_down = monte_carlo_call(S0 - h, K, r, sigma, T, N, rng);

    return (price_up - price_down) / (2.0 * h);
}

double monte_carlo_call_antithetic(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);
    int half_N = N / 2;
    double payoff_sum = 0.0;

    for (int i = 0; i < half_N; ++i)
    {
        double Z = dist(rng);

        double ST_pos = simulate_terminal_price(S0, r, sigma, T, Z);
        double ST_neg = simulate_terminal_price(S0, r, sigma, T, -Z);

        payoff_sum += 0.5 * (std::max(ST_pos - K, 0.0) +
                             std::max(ST_neg - K, 0.0));
    }

    return std::exp(-r * T) * (payoff_sum / half_N);
}

MCResult monte_carlo_call_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);

    double payoff_sum = 0.0;
    double delta_sum = 0.0;

    double drift = (r - 0.5 * sigma * sigma) * T;
    double diffusion = sigma * std::sqrt(T);

    for (int i = 0; i < N; ++i)
    {
        double Z = dist(rng);
        double ST = S0 * std::exp(drift + diffusion * Z);

        double payoff = std::max(ST - K, 0.0);
        payoff_sum += payoff;

        if (ST > K)
        {
            delta_sum += ST / S0;
        }
    }

    MCResult result;
    result.price = std::exp(-r * T) * (payoff_sum / N);
    result.delta = std::exp(-r * T) * (delta_sum / N);

    return result;
}

MCResult monte_carlo_call_antithetic_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);

    int half_N = N / 2;
    double payoff_sum = 0.0;
    double delta_sum = 0.0;

    double drift = (r - 0.5 * sigma * sigma) * T;
    double diffusion = sigma * std::sqrt(T);
    double discount = std::exp(-r * T);

    for (int i = 0; i < half_N; ++i)
    {
        double Z = dist(rng);

        double ST_pos = S0 * std::exp(drift + diffusion * Z);
        double ST_neg = S0 * std::exp(drift - diffusion * Z);

        double payoff_pos = std::max(ST_pos - K, 0.0);
        double payoff_neg = std::max(ST_neg - K, 0.0);

        payoff_sum += 0.5 * (payoff_pos + payoff_neg);

        // Pathwise delta (only contributes if ITM)
        if (ST_pos > K)
            delta_sum += 0.5 * (ST_pos / S0);
        if (ST_neg > K)
            delta_sum += 0.5 * (ST_neg / S0);
    }

    MCResult result;
    result.price = discount * (payoff_sum / half_N);
    result.delta = discount * (delta_sum / half_N);

    return result;
}
