#ifndef MC_PRICER_H
#define MC_PRICER_H

#include <vector>
#include <random>

// result container for monte carlo pricing - groups option price and delta
struct MCResult
{
    double price;
    double delta;

    double std_error; // standard error of price estimate
    double ci_lower;  // 95% confidence interval lower bound
    double ci_upper;  // 95% confidence interval upper bound
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

// -----------------------------
// Black–Scholes analytical pricing
// -----------------------------

double black_scholes_call_price(
    double S0,
    double K,
    double r,
    double sigma,
    double T);

double black_scholes_call_vega(
    double S0,
    double K,
    double r,
    double sigma,
    double T);

// -----------------------------
// Implied volatility solver
// -----------------------------

double implied_volatility_call(
    double market_price,
    double S0,
    double K,
    double r,
    double T,
    double initial_guess = 0.2,
    int max_iterations = 100,
    double tolerance = 1e-8);

// ============================================================
// Trade Evaluation Statistics (Real-World Simulation)
// ============================================================

struct MCTradeStats
{
    double expected_pnl;
    double prob_profit;
    double prob_itm;
    double prob_breakeven;

    std::vector<double> pnl_paths; // FULL simulated PnL distribution
};

MCTradeStats monte_carlo_trade_stats(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    double mu,      // real-world drift
    double premium, // price paid for option
    int N,
    std::mt19937 &rng);

#endif