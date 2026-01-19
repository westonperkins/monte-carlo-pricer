import sys
import os

# add build directory to Python path
sys.path.append(
    os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
)

import mc_pricer_py
from black_scholes import call_price


S0 = 100
K = 105
r = 0.05
sigma = 0.2
T = 1.0
N = 500_000

mc = mc_pricer_py.call_price(S0, K, r, sigma, T, N)
bs = call_price(S0, K, r, sigma, T)

print("Monte Carlo :", mc)
print("Black–Scholes:", bs)
print("Difference  :", mc - bs)
