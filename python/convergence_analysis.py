import sys
import os
import numpy as np
import matplotlib.pyplot as plt

# Ensure Python can locate compiled module
sys.path.append(os.path.abspath("../build"))

import mc_pricer_py as mc
from black_scholes import call_price

# -----------------------------
# Option Parameters
# -----------------------------

S0 = 100.0
K = 105.0
r = 0.05
sigma = 0.2
T = 1.0

bs_price = call_price(S0, K, r, sigma, T)

Ns = np.array([1_000, 5_000, 10_000, 50_000, 100_000, 500_000])

# Containers
mc_std = []
mc_ant = []

ci_std_lower = []
ci_std_upper = []

ci_ant_lower = []
ci_ant_upper = []

# -----------------------------
# Run Experiments
# -----------------------------

for N in Ns:
    print(f"Running N={N}...")

    res_std = mc.call_price_full(S0, K, r, sigma, T, int(N))
    res_ant = mc.call_price_full_antithetic(S0, K, r, sigma, T, int(N))

    mc_std.append(res_std.price)
    ci_std_lower.append(res_std.ci_lower)
    ci_std_upper.append(res_std.ci_upper)

    mc_ant.append(res_ant.price)
    ci_ant_lower.append(res_ant.ci_lower)
    ci_ant_upper.append(res_ant.ci_upper)

mc_std = np.array(mc_std)
mc_ant = np.array(mc_ant)

ci_std_lower = np.array(ci_std_lower)
ci_std_upper = np.array(ci_std_upper)

ci_ant_lower = np.array(ci_ant_lower)
ci_ant_upper = np.array(ci_ant_upper)

# -----------------------------
# Plot 1: Convergence + CI Bands
# -----------------------------

plt.figure()

plt.plot(Ns, mc_std, "o-", label="Standard MC")
plt.fill_between(Ns, ci_std_lower, ci_std_upper, alpha=0.2)

plt.plot(Ns, mc_ant, "o-", label="Antithetic MC")
plt.fill_between(Ns, ci_ant_lower, ci_ant_upper, alpha=0.2)

plt.axhline(bs_price, linestyle="--", label="Black–Scholes")

plt.xscale("log")
plt.xlabel("Number of paths (N)")
plt.ylabel("Option price")
plt.title("Monte Carlo Convergence with 95% CI")
plt.legend()
plt.tight_layout()
plt.show()

# -----------------------------
# Plot 2: CI Width Decay
# -----------------------------

plt.figure()

ci_width_std = ci_std_upper - ci_std_lower
ci_width_ant = ci_ant_upper - ci_ant_lower

plt.plot(Ns, ci_width_std, "o-", label="Standard CI width")
plt.plot(Ns, ci_width_ant, "o-", label="Antithetic CI width")

theory = ci_width_std[0] * np.sqrt(Ns[0] / Ns)
plt.plot(Ns, theory, "--", label="~ 1/sqrt(N)")

plt.xscale("log")
plt.yscale("log")
plt.xlabel("Number of paths (N)")
plt.ylabel("Confidence Interval Width")
plt.title("Confidence Interval Decay")
plt.legend()
plt.tight_layout()
plt.show()

# -----------------------------
# Plot 3: Absolute Pricing Error
# -----------------------------

plt.figure()

plt.plot(Ns, np.abs(mc_std - bs_price), "o-", label="Standard MC error")
plt.plot(Ns, np.abs(mc_ant - bs_price), "o-", label="Antithetic MC error")

plt.xscale("log")
plt.yscale("log")
plt.xlabel("Number of paths (N)")
plt.ylabel("Absolute error")
plt.title("Monte Carlo Error Decay")
plt.legend()
plt.tight_layout()
plt.show()