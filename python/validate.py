import sys
import os

# monte carlo price validation
# script validates monte carlo options price computed by the c++ engine against the analytical black scholes price
# purpose:
# - verify correctness of the monte carlo simulator
# - ensure no bias in the stochastic pricing logic
# - establish a ground thrith benchmark

# if the monte carlo engine is implemented correctly, the difference between the two prices should shrink as the number of simulated paths increases

# ensure python can find the compiled C++ module
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build")))

import mc_pricer_py
from black_scholes import call_price

# option parameters (european call options)
S0 = 100
K = 105
r = 0.05
sigma = 0.2
T = 1.0
N = 500_000

# monte carlo price (C++ backend)
mc = mc_pricer_py.call_price(S0, K, r, sigma, T, N)
# analytical black scholes price
bs = call_price(S0, K, r, sigma, T)

print("Monte Carlo :", mc)
print("Black–Scholes:", bs)
print("Difference  :", mc - bs)
