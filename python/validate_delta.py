import sys, os
sys.path.append(os.path.abspath("../build"))

import mc_pricer_py
from black_scholes import call_delta

S0, K, r, sigma, T = 100, 105, 0.05, 0.2, 1.0
N = 300_000

mc_delta = mc_pricer_py.delta(S0, K, r, sigma, T, N)
bs_delta = call_delta(S0, K, r, sigma, T)

print("MC Delta :", mc_delta)
print("BS Delta :", bs_delta)
print("Difference:", mc_delta - bs_delta)
