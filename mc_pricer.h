#ifndef MC_PRICER_H
#define MC_PRICER_H

#include <random>

// result container for monte carlo pricing - groups option price and delta
struct MCResult
{
    double price;
    double delta;
};

// standard monte carlo pricing (call) - estimates price of european call option (no greeks)
double monte_carlo_call(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng); // random number generator

// antithetic monte carlo pricing (call) - uses paired simulations to reduce randomness and improve convergence w/o increasing runtime
double monte_carlo_call_antithetic(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng);

// finite difference delta (diagnostic) - approximates delta by slightly perturbing the stock price and re running the simulation
double monte_carlo_delta(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h, // size of price perturbation
    std::mt19937 &rng);

// single pass monte carlo price and delta - computes both the option price and delta in one simulation pass w pathwise differentiation 
MCResult monte_carlo_call_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng);

// antithetic single pass monte carlo price and delta  - most accurate and efficient
MCResult monte_carlo_call_antithetic_with_greeks(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    std::mt19937 &rng);

// generic payoff-based pricer  - prices a european stle derivative using an abstract payoff definition. able to price calls, puts, and future exotic contracts w out changing logic
class Payoff;
double monte_carlo_price(
    double S0,
    double r,
    double sigma,
    double T,
    int N,
    const Payoff &payoff, // use defined payoff logic
    std::mt19937 &rng);

#endif