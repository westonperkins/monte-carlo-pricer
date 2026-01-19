import sys
import os
sys.path.append(os.path.abspath("../build"))

import mc_pricer_py
from black_scholes import call_price

S0, K, r, sigma, T = 100, 105, 0.05, 0.2, 1.0

paths = [1_000, 5_000, 20_000, 100_000, 500_000]
bs = call_price(S0, K, r, sigma, T)

print(f"Black–Scholes: {bs:.4f}\n")

for N in paths:
    mc = mc_pricer_py.call_price(S0, K, r, sigma, T, N)
    print(f"N={N:>7}  MC={mc:.4f}  Error={mc - bs:+.4f}")
