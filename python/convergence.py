import sys
import os

# lightewight monte carlo convergence check
# script provides a quick text based validation of the monte carlo rcing engine
# unlike the full plotting script, this version
# - prints results directly to the console
# - is fast to run
# - is useful for sanity checks during development
# compares monte carlo prices against the analytical black scholes benchmark

# ensure python can locate the compiled c++ module
# the monte carlo engine is compiled into ../build
sys.path.append(os.path.abspath("../build"))

import mc_pricer_py
from black_scholes import call_price

# option parameters (euro call)
S0, K, r, sigma, T = 100, 105, 0.05, 0.2, 1.0

# monte carlo path counts to test
# increasing number of simulated paths lets us observe how monte carlo estimates converge over time
paths = [1_000, 5_000, 20_000, 100_000, 500_000]

# analytical benchmark (ground truth)
# black scholes provides the exact solution for european options which monte carlo should approach
bs = call_price(S0, K, r, sigma, T)

print(f"Black–Scholes: {bs:.4f}\n")


# run monte carlo experiments
for N in paths:
    # monte carlo estimate using C++ backend
    mc = mc_pricer_py.call_price(S0, K, r, sigma, T, N)

    # print pricing error relative to black scholes
    print(f"N={N:>7}  MC={mc:.4f}  Error={mc - bs:+.4f}")
