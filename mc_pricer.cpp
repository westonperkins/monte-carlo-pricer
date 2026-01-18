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
    int N
) {
    std::mt19937 gen(42);
    std::normal_distribution<> dist(0.0, 1.0);

    double payoff_sum = 0.0;

    for (int i = 0; i < N; ++i) {
        double Z = dist(gen);

        double ST = S0 * std::exp(
            (r - 0.5 * sigma * sigma) * T
            + sigma * std::sqrt(T) * Z
        );

        payoff_sum += std::max(ST - K, 0.0);
    }

    return std::exp(-r * T) * (payoff_sum / N);
}
