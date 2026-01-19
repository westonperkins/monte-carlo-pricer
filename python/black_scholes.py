import math
from math import log, sqrt, exp
from statistics import NormalDist

N = NormalDist()

def call_price(S, K, r, sigma, T):
    if sigma == 0:
        return max(S * exp(r*T) - K, 0) * exp(-r*T)

    d1 = (log(S / K) + (r + 0.5 * sigma**2) * T) / (sigma * sqrt(T))
    d2 = d1 - sigma * sqrt(T)

    return (
        S * N.cdf(d1)
        - K * exp(-r * T) * N.cdf(d2)
    )
