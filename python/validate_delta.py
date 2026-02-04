import sys, os

# monte carlo delta validation
# this script validates the monte carlo estimate of the option DELTA against the analytical black scholes delta
# delta measures sensitivity of the option relative to the underlying asset price

# this is an important diagnostic bc
# - greeks are harder to estimate than prices
# - small numerical errors can amplify
# - it verifies correct pathwise differentiation
sys.path.append(os.path.abspath("../build"))

# ensure python can find the compiled C++
import mc_pricer_py
from black_scholes import call_delta

# option parameters (euro call)
S0, K, r, sigma, T = 100, 105, 0.05, 0.2, 1.0
N = 300_000

# monte carlo delta estimate
# computed using the C++ backend w pathwise differentiation
mc_delta = mc_pricer_py.delta(S0, K, r, sigma, T, N)
# analytical black scholes delta
bs_delta = call_delta(S0, K, r, sigma, T)

print("MC Delta :", mc_delta)
print("BS Delta :", bs_delta)
print("Difference:", mc_delta - bs_delta)
