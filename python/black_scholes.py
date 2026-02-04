import math
from math import log, sqrt, exp
from statistics import NormalDist

# Black-scholes analytical pricing formulas
# provides closed form solutions for pricing european call options and computing their delta
# these formulas used as:
# 1. a benchmark to validate monte carlo results
# 2. a reference model for understanding convergence
# 3. a sanity check for greeks
# black scholes is purely mathematical and doesnt rely on random simulations
N = NormalDist()


# standard normal distribution used to evaluate probabilities under a normal curve
def call_price(S, K, r, sigma, T):
    # edge case: zero volatility
    # if there is no randomness in the stock price the option payoff is deterministic
    if sigma == 0:
        return max(S * exp(r * T) - K, 0) * exp(-r * T)

    # core black scholes terms
    # d1 and d2 measure how far the option is "in the money" in standardized normal units
    d1 = (log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * sqrt(T))
    d2 = d1 - sigma * sqrt(T)
    # final black scholes pricing formula
    # interpreted as expected stock payoff minus discounted strike cost
    return S * N.cdf(d1) - K * exp(-r * T) * N.cdf(d2)


# black scholes delta - sensitivity to stock price
def call_delta(S, K, r, sigma, T):
    # edge case zero volatility
    # if the outcome is deterministic, delta is either 0 or 1 depending on whether the option finishes in the money
    if sigma == 0:
        return 1.0 if S * exp(r * T) > K else 0.0
    # delta formula
    # for a european call option delta is simply the probability that the option finishes in the moeny under the risk neutral measure
    d1 = (log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * sqrt(T))
    return N.cdf(d1)
