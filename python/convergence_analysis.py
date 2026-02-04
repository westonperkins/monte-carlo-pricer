import sys
import os
import numpy as np
import matplotlib.pyplot as plt

# convergence and variance reduction analysis
# script compares monte carlo option pricing against the analytical black scholes price
# goals:
# 1. demonstrate monte carlo convergence as n increases
# 2. compare standard MC vs antithetic variates
# 3. visualize error decay on a log scale
# validation and diagnostics tool, not part of the pricing engine itself

# ensure python can locate the compiled C++ extension
sys.path.append(os.path.abspath("../build"))

import mc_pricer_py as mc
from black_scholes import call_price


# option parameters
S0 = 100.0
K = 105.0
r = 0.05
sigma = 0.2
T = 1.0

# ground truth (analyitcal benchmark)
# black scholes provides an exact solution for european options - monte carlo estimates should converge to this
bs_price = call_price(S0, K, r, sigma, T)

# monte carlo experiment sizes
# increasing number of simulated price paths to observe convergence behavior
Ns = np.array([1_000, 5_000, 10_000, 50_000, 100_000, 500_000])

# containers for results
mc_std = []  # standard monte carlo estimates
mc_ant = []  # antithetic variates estimates

# run monte carlo experiments
for N in Ns:
    print(f"Running N={N}...")

    # standard monte carlo pricing
    price_std = mc.call_price(S0, K, r, sigma, T, int(N))
    # monte carlo with antithetic ariance reduction
    price_ant = mc.call_price_antithetic(S0, K, r, sigma, T, int(N))

    mc_std.append(price_std)
    mc_ant.append(price_ant)

# convert results to NumPy arrays for analysis
mc_std = np.array(mc_std)
mc_ant = np.array(mc_ant)

# plot 1: price convergence
plt.figure()
plt.plot(Ns, mc_std, "o-", label="Standard MC")
plt.plot(Ns, mc_ant, "o-", label="Antithetic MC")
plt.axhline(bs_price, linestyle="--", label="Black–Scholes")

plt.xscale("log")
plt.xlabel("Number of paths (N)")
plt.ylabel("Option price")
plt.title("Monte Carlo convergence")
plt.legend()
plt.tight_layout()
plt.show()

# plot 2: absolute pricing error
plt.figure()
plt.plot(Ns, np.abs(mc_std - bs_price), "o-", label="Standard MC error")
plt.plot(Ns, np.abs(mc_ant - bs_price), "o-", label="Antithetic MC error")

plt.xscale("log")
plt.yscale("log")
plt.xlabel("Number of paths (N)")
plt.ylabel("Absolute error")
plt.title("Monte Carlo error decay")
plt.legend()
plt.tight_layout()
plt.show()
