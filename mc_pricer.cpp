#include "mc_pricer.h"
#include <random>
#include <cmath>
#include <algorithm>

double monte_carlo_call(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937& rng
) {
    std::normal_distribution<> dist(0.0, 1.0);

    double payoff_sum = 0.0;

    for (int i = 0; i < N; ++i) {
        double Z = dist(rng);

        double ST = S0 * std::exp(
            (r - 0.5 * sigma * sigma) * T
            + sigma * std::sqrt(T) * Z
        );

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
    std::mt19937& rng
) {
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
    std::mt19937& rng
) {
    std::normal_distribution<> dist(0.0, 1.0);

    int half_N = N / 2;
    double payoff_sum = 0.0;

    double drift = (r - 0.5 * sigma * sigma) * T;
    double diffusion = sigma * std::sqrt(T);

    for (int i = 0; i < half_N; ++i) {
        double Z = dist(rng);

        double ST_pos = S0 * std::exp(drift + diffusion * Z);
        double ST_neg = S0 * std::exp(drift - diffusion * Z);

        payoff_sum += 0.5 * (
            std::max(ST_pos - K, 0.0) +
            std::max(ST_neg - K, 0.0)
        );
    }

    return std::exp(-r * T) * (payoff_sum / half_N);
}
