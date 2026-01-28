#ifndef MC_PRICER_H
#define MC_PRICER_H

#include <random>

struct MCResult
{
    double price;
    double delta;
};

// Plain Monte Carlo
double monte_carlo_call(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937& rng);

// Antithetic Monte Carlo
double monte_carlo_call_antithetic(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937& rng);

// Finite-difference Delta (diagnostic)
double monte_carlo_delta(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h,
    std::mt19937& rng);

// Single-pass price + delta
MCResult monte_carlo_call_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937& rng);

// Antithetic single-pass price + delta
MCResult monte_carlo_call_antithetic_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937& rng);

// Generic payoff-based pricer
class Payoff;
double monte_carlo_price(
    double S0,
    double r,
    double sigma,
    double T,
    int N,
    const Payoff& payoff,
    std::mt19937& rng);

#endif