import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build")))
import mc_pricer_py as mc


def plot_simulation_paths(S0, r, sigma, T, K, strike_label, num_paths=200, steps=252):
    flat = mc.simulate_paths(S0, r, sigma, T, num_paths, steps)
    paths = np.array(flat).reshape(num_paths, steps + 1)
    t = np.linspace(0, T, steps + 1)

    terminal = paths[:, -1]
    itm_count = np.sum(terminal > K)
    otm_count = num_paths - itm_count

    fig, axes = plt.subplots(1, 2, figsize=(16, 7), gridspec_kw={"width_ratios": [2, 1]})

    # left: simulation paths
    ax = axes[0]
    for i in range(num_paths):
        color = "#2ecc71" if terminal[i] > K else "#e74c3c"
        ax.plot(t, paths[i], color=color, alpha=0.15, linewidth=0.5)

    ax.axhline(K, color="white", linestyle="--", linewidth=1.5, label=f"Strike ${K:.2f}")
    ax.axhline(S0, color="#3498db", linestyle="-", linewidth=1.0, alpha=0.6, label=f"Spot ${S0:.2f}")

    ax.set_xlabel("Time (years)", fontsize=12)
    ax.set_ylabel("Stock Price ($)", fontsize=12)
    ax.set_title(f"Monte Carlo Simulation — {num_paths} Paths\n{strike_label}", fontsize=13)
    ax.legend(loc="upper left", fontsize=10)
    ax.set_facecolor("#1a1a2e")
    ax.tick_params(colors="white")

    # right: terminal distribution
    ax2 = axes[1]
    ax2.hist(terminal, bins=60, orientation="horizontal", color="#3498db", alpha=0.7, edgecolor="none")
    ax2.axhline(K, color="white", linestyle="--", linewidth=1.5)
    ax2.axhline(S0, color="#3498db", linestyle="-", linewidth=1.0, alpha=0.6)

    ax2.set_xlabel("Frequency", fontsize=12)
    ax2.set_title("Terminal Distribution", fontsize=13)
    ax2.set_facecolor("#1a1a2e")
    ax2.tick_params(colors="white")

    ax2.text(
        0.95, 0.95,
        f"ITM: {itm_count} ({100*itm_count/num_paths:.1f}%)\nOTM: {otm_count} ({100*otm_count/num_paths:.1f}%)",
        transform=ax2.transAxes, fontsize=11, verticalalignment="top",
        horizontalalignment="right", color="white",
        bbox=dict(boxstyle="round,pad=0.4", facecolor="#16213e", alpha=0.8),
    )

    fig.patch.set_facecolor("#0f0f23")
    plt.tight_layout()
    return fig


def plot_valuation_summary(ticker, option_type, strike, expiration,
                           market_price, mc_price, bs_price,
                           mc_result, contract_info, S0, sigma, r, T):
    fig = plt.figure(figsize=(14, 8))
    fig.patch.set_facecolor("#0f0f23")

    gs = gridspec.GridSpec(2, 2, hspace=0.35, wspace=0.3)

    # --- top left: price comparison bar chart ---
    ax1 = fig.add_subplot(gs[0, 0])
    labels = ["Market Price", "MC Fair Value", "BS Fair Value"]
    values = [market_price, mc_price, bs_price]
    colors = ["#e74c3c", "#2ecc71", "#3498db"]
    bars = ax1.bar(labels, values, color=colors, width=0.5, edgecolor="none")
    for bar, val in zip(bars, values):
        ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.02*max(values),
                 f"${val:.4f}", ha="center", va="bottom", color="white", fontsize=11, fontweight="bold")
    ax1.set_ylabel("Option Price ($)", fontsize=11)
    ax1.set_title("Price Comparison", fontsize=13, color="white")
    ax1.set_facecolor("#1a1a2e")
    ax1.tick_params(colors="white")
    ax1.spines["bottom"].set_color("white")
    ax1.spines["left"].set_color("white")

    # --- top right: valuation verdict ---
    ax2 = fig.add_subplot(gs[0, 1])
    ax2.set_facecolor("#1a1a2e")
    ax2.set_xlim(0, 1)
    ax2.set_ylim(0, 1)
    ax2.axis("off")

    diff_mc = mc_price - market_price
    diff_pct = (diff_mc / market_price) * 100 if market_price > 0 else 0

    if diff_pct > 5:
        verdict = "UNDERVALUED"
        verdict_color = "#2ecc71"
        verdict_desc = f"MC fair value is {diff_pct:.1f}% above market price"
    elif diff_pct < -5:
        verdict = "OVERVALUED"
        verdict_color = "#e74c3c"
        verdict_desc = f"MC fair value is {abs(diff_pct):.1f}% below market price"
    else:
        verdict = "FAIRLY PRICED"
        verdict_color = "#f39c12"
        verdict_desc = f"MC fair value is within {abs(diff_pct):.1f}% of market price"

    ax2.text(0.5, 0.85, f"{ticker.upper()} ${strike} {option_type.upper()}",
             ha="center", va="center", fontsize=14, color="white", fontweight="bold",
             transform=ax2.transAxes)
    ax2.text(0.5, 0.65, verdict,
             ha="center", va="center", fontsize=28, color=verdict_color, fontweight="bold",
             transform=ax2.transAxes)
    ax2.text(0.5, 0.48, verdict_desc,
             ha="center", va="center", fontsize=11, color="#bdc3c7",
             transform=ax2.transAxes)
    ax2.text(0.5, 0.30, f"Exp: {expiration}  |  IV: {sigma*100:.1f}%  |  r: {r*100:.2f}%",
             ha="center", va="center", fontsize=10, color="#95a5a6",
             transform=ax2.transAxes)
    ax2.text(0.5, 0.15, f"Spot: ${S0:.2f}  |  T: {T:.4f}y  |  Bid/Ask: ${contract_info['bid']:.2f}/${contract_info['ask']:.2f}",
             ha="center", va="center", fontsize=10, color="#95a5a6",
             transform=ax2.transAxes)

    # --- bottom left: MC convergence info ---
    ax3 = fig.add_subplot(gs[1, 0])
    ax3.set_facecolor("#1a1a2e")
    ax3.axis("off")

    info_lines = [
        ("MC Price", f"${mc_price:.4f}"),
        ("95% CI", f"[${mc_result.ci_lower:.4f}, ${mc_result.ci_upper:.4f}]"),
        ("Std Error", f"${mc_result.std_error:.6f}"),
        ("Delta", f"{mc_result.delta:.4f}"),
        ("BS Price", f"${bs_price:.4f}"),
        ("MC - Market", f"${diff_mc:+.4f} ({diff_pct:+.1f}%)"),
    ]

    ax3.text(0.5, 0.95, "Pricing Details", ha="center", va="top",
             fontsize=13, color="white", fontweight="bold", transform=ax3.transAxes)
    for i, (label, value) in enumerate(info_lines):
        y = 0.80 - i * 0.13
        ax3.text(0.15, y, label, ha="left", va="center", fontsize=11, color="#95a5a6",
                 transform=ax3.transAxes)
        ax3.text(0.85, y, value, ha="right", va="center", fontsize=11, color="white",
                 fontweight="bold", transform=ax3.transAxes)

    # --- bottom right: contract info ---
    ax4 = fig.add_subplot(gs[1, 1])
    ax4.set_facecolor("#1a1a2e")
    ax4.axis("off")

    contract_lines = [
        ("Last Price", f"${contract_info['lastPrice']:.2f}"),
        ("Bid / Ask", f"${contract_info['bid']:.2f} / ${contract_info['ask']:.2f}"),
        ("Implied Vol", f"{contract_info['impliedVolatility']*100:.1f}%"),
        ("Volume", f"{contract_info['volume']:,}"),
        ("Open Interest", f"{contract_info['openInterest']:,}"),
        ("In The Money", "Yes" if contract_info["inTheMoney"] else "No"),
    ]

    ax4.text(0.5, 0.95, "Contract Details", ha="center", va="top",
             fontsize=13, color="white", fontweight="bold", transform=ax4.transAxes)
    for i, (label, value) in enumerate(contract_lines):
        y = 0.80 - i * 0.13
        ax4.text(0.15, y, label, ha="left", va="center", fontsize=11, color="#95a5a6",
                 transform=ax4.transAxes)
        ax4.text(0.85, y, value, ha="right", va="center", fontsize=11, color="white",
                 fontweight="bold", transform=ax4.transAxes)

    plt.suptitle("Monte Carlo Options Analyzer", fontsize=16, color="white", y=0.98)
    return fig
