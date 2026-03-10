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

        double mean = 0.0;
        double m2 = 0.0; // sum of squares of differences
        double delta_sum = 0.0;

        for (int i = 0; i < N; ++i)
        {
            double Z = dist(rng);

            double p = payoff(Z);
            double d = delta_contrib(Z);

            // --- Welford update ---
            double delta_mean = p - mean;
            mean += delta_mean / (i + 1);
            m2 += delta_mean * (p - mean);

            delta_sum += d;
        }

        double variance = (N > 1) ? (m2 / (N - 1)) : 0.0;
        double std_error = std::sqrt(variance / N);

        double discount = std::exp(-r * T);

        MCResult result;
        result.price = discount * mean;
        result.delta = discount * (delta_sum / N);

        result.std_error = discount * std_error;

        double ci_half_width = 1.96 * result.std_error;
        result.ci_lower = result.price - ci_half_width;
        result.ci_upper = result.price + ci_half_width;

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
    std::mt19937 rng_up = rng;
    std::mt19937 rng_down = rng;

    double price_up =
        monte_carlo_call(S0 + h, K, r, sigma, T, N, rng_up);

    double price_down =
        monte_carlo_call(S0 - h, K, r, sigma, T, N, rng_down);

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

// single pass monte carlo put price and delta
MCResult monte_carlo_put_with_greeks(
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
        return std::max(K - ST, 0.0);
    };

    auto delta_contrib = [&](double Z)
    {
        double ST = simulate_terminal_price(S0, r, sigma, T, Z);
        return (ST < K) ? -(ST / S0) : 0.0;
    };

    return monte_carlo_engine(
        N, r, T, rng, payoff, delta_contrib);
}

// antithetic single pass monte carlo put price and delta
MCResult monte_carlo_put_antithetic_with_greeks(
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

        return 0.5 * (std::max(K - ST_pos, 0.0) +
                      std::max(K - ST_neg, 0.0));
    };

    auto delta_contrib = [&](double Z)
    {
        double ST_pos = simulate_terminal_price(S0, r, sigma, T, Z);
        double ST_neg = simulate_terminal_price(S0, r, sigma, T, -Z);

        double d = 0.0;
        if (ST_pos < K)
            d += 0.5 * (-(ST_pos / S0));
        if (ST_neg < K)
            d += 0.5 * (-(ST_neg / S0));
        return d;
    };

    return monte_carlo_engine(
        half_N, r, T, rng, payoff, delta_contrib);
}

// simulate full GBM paths for visualization
std::vector<double> simulate_paths(
    double S0,
    double r,
    double sigma,
    double T,
    int N,
    int steps,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);

    double dt = T / steps;
    double drift = (r - 0.5 * sigma * sigma) * dt;
    double diffusion = sigma * std::sqrt(dt);

    // row-major: path i, step j -> index i * (steps+1) + j
    std::vector<double> paths(N * (steps + 1));

    for (int i = 0; i < N; ++i)
    {
        int base = i * (steps + 1);
        paths[base] = S0;

        for (int j = 1; j <= steps; ++j)
        {
            double Z = dist(rng);
            paths[base + j] = paths[base + j - 1] *
                               std::exp(drift + diffusion * Z);
        }
    }

    return paths;
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

// ============================================================
// Black–Scholes Analytical Pricing (Call)
// ============================================================

namespace
{
    inline double normal_cdf(double x)
    {
        return 0.5 * std::erfc(-x / std::sqrt(2));
    }

    inline double normal_pdf(double x)
    {
        static const double INV_SQRT_2PI = 0.3989422804014327;
        return INV_SQRT_2PI * std::exp(-0.5 * x * x);
    }
}

double black_scholes_call_price(
    double S0,
    double K,
    double r,
    double sigma,
    double T)
{
    if (sigma <= 0.0 || T <= 0.0)
        return std::max(S0 - K, 0.0);

    double sqrtT = std::sqrt(T);

    double d1 = (std::log(S0 / K) +
                 (r + 0.5 * sigma * sigma) * T) /
                (sigma * sqrtT);

    double d2 = d1 - sigma * sqrtT;

    return S0 * normal_cdf(d1) - K * std::exp(-r * T) * normal_cdf(d2);
}

double black_scholes_put_price(
    double S0,
    double K,
    double r,
    double sigma,
    double T)
{
    if (sigma <= 0.0 || T <= 0.0)
        return std::max(K - S0, 0.0);

    double sqrtT = std::sqrt(T);

    double d1 = (std::log(S0 / K) +
                 (r + 0.5 * sigma * sigma) * T) /
                (sigma * sqrtT);

    double d2 = d1 - sigma * sqrtT;

    return K * std::exp(-r * T) * normal_cdf(-d2) - S0 * normal_cdf(-d1);
}

double black_scholes_call_vega(
    double S0,
    double K,
    double r,
    double sigma,
    double T)
{
    if (sigma <= 0.0 || T <= 0.0)
        return 0.0;

    double sqrtT = std::sqrt(T);

    double d1 = (std::log(S0 / K) +
                 (r + 0.5 * sigma * sigma) * T) /
                (sigma * sqrtT);

    return S0 * sqrtT * normal_pdf(d1);
}

// ============================================================
// Implied Volatility Solver (Newton–Raphson)
// ============================================================

double implied_volatility_call(
    double market_price,
    double S0,
    double K,
    double r,
    double T,
    double initial_guess,
    int max_iterations,
    double tolerance)
{
    double sigma = initial_guess;

    for (int i = 0; i < max_iterations; ++i)
    {
        double price = black_scholes_call_price(
            S0, K, r, sigma, T);

        double diff = price - market_price;

        if (std::abs(diff) < tolerance)
            return sigma;

        double vega = black_scholes_call_vega(
            S0, K, r, sigma, T);

        if (vega < 1e-8)
            break;

        sigma -= diff / vega;

        // enforce reasonable bounds
        if (sigma < 1e-6)
            sigma = 1e-6;
        if (sigma > 5.0)
            sigma = 5.0;
    }

    return sigma; // return best estimate if not fully converged
}

// ============================================================
// Trade Evaluation Engine (Real-World Drift)
// ============================================================

MCTradeStats monte_carlo_trade_stats(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    double mu,
    double premium,
    int N,
    std::mt19937 &rng)
{
    std::normal_distribution<> dist(0.0, 1.0);

    double expected_pnl = 0.0;
    int count_profit = 0;
    int count_itm = 0;
    int count_breakeven = 0;

    double drift = (mu - 0.5 * sigma * sigma) * T;
    double diffusion = sigma * std::sqrt(T);

    MCTradeStats stats;

    // Reserve memory once (important for performance)
    stats.pnl_paths.reserve(N);

    for (int i = 0; i < N; ++i)
    {
        double Z = dist(rng);
        double ST = S0 * std::exp(drift + diffusion * Z);

        double payoff = std::max(ST - K, 0.0);
        double pnl = payoff - premium;

        // Store full distribution
        stats.pnl_paths.push_back(pnl);

        expected_pnl += pnl;

        if (pnl > 0.0)
            count_profit++;

        if (ST > K)
            count_itm++;

        if (ST > K + premium)
            count_breakeven++;
    }

    stats.expected_pnl = expected_pnl / N;
    stats.prob_profit = static_cast<double>(count_profit) / N;
    stats.prob_itm = static_cast<double>(count_itm) / N;
    stats.prob_breakeven = static_cast<double>(count_breakeven) / N;

    return stats;
}