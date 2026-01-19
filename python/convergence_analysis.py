import sys
import os
import numpy as np
import matplotlib.pyplot as plt

# Ensure Python can find the compiled module
sys.path.append(os.path.abspath("../build"))

import mc_pricer_py as mc
from black_scholes import call_price

# -----------------------------
# Option parameters
# -----------------------------
S0 = 100.0
K = 105.0
r = 0.05
sigma = 0.2
T = 1.0

# Ground truth (Black–Scholes)
bs_price = call_price(S0, K, r, sigma, T)

# Monte Carlo path counts
Ns = np.array([1_000, 5_000, 10_000, 50_000, 100_000, 500_000])

mc_std = []
mc_ant = []

# -----------------------------
# Run Monte Carlo experiments
# -----------------------------
for N in Ns:
    print(f"Running N={N}...")

    price_std = mc.call_price(S0, K, r, sigma, T, int(N))
    price_ant = mc.call_price_antithetic(S0, K, r, sigma, T, int(N))

    mc_std.append(price_std)
    mc_ant.append(price_ant)

mc_std = np.array(mc_std)
mc_ant = np.array(mc_ant)

# -----------------------------
# Plot 1: Convergence
# -----------------------------
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

# -----------------------------
# Plot 2: Absolute error
# -----------------------------
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
