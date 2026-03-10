#!/usr/bin/env python3
"""
Monte Carlo Options Analyzer
Fetches real option contracts, prices them with MC simulation,
compares to market price, and visualizes simulation paths.
"""

import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build")))

import mc_pricer_py as mc
from market_data import (
    get_stock_price,
    get_expirations,
    get_option_chain,
    fetch_contract,
    get_risk_free_rate,
    time_to_expiry,
    estimate_historical_mu,
)
from visualizer import plot_simulation_paths, plot_valuation_summary
from risk_metrics import compute_risk_metrics
import matplotlib.pyplot as plt


def select_from_list(items, prompt_text):
    for i, item in enumerate(items):
        print(f"  [{i}] {item}")
    while True:
        try:
            idx = int(input(f"\n{prompt_text}: "))
            if 0 <= idx < len(items):
                return items[idx]
            print(f"Enter a number between 0 and {len(items)-1}")
        except (ValueError, EOFError):
            print("Invalid input, enter a number")


def display_strikes(chain, S0):
    strikes = sorted(chain["strike"].tolist())
    print(f"\n  Available strikes (spot=${S0:.2f}):")
    for i, s in enumerate(strikes):
        itm = " <-- ATM" if abs(s - S0) / S0 < 0.02 else ""
        print(f"  [{i}] ${s:.2f}{itm}")
    return strikes


def run_analysis(ticker, expiration, strike, option_type="call", num_sims=500_000, num_viz_paths=200):
    print(f"\n{'='*60}")
    print(f"  MONTE CARLO OPTIONS ANALYZER")
    print(f"  {ticker.upper()} ${strike} {option_type.upper()} — Exp: {expiration}")
    print(f"{'='*60}")

    # fetch market data
    print("\nFetching market data...")
    S0 = get_stock_price(ticker)
    r = get_risk_free_rate()
    T = time_to_expiry(expiration)
    contract = fetch_contract(ticker, expiration, strike, option_type)

    market_price = contract["lastPrice"]
    sigma = contract["impliedVolatility"]

    print(f"  Spot Price:     ${S0:.2f}")
    print(f"  Market Price:   ${market_price:.2f}")
    print(f"  Implied Vol:    {sigma*100:.1f}%")
    print(f"  Risk-Free Rate: {r*100:.2f}%")
    print(f"  Time to Expiry: {T:.4f} years ({int(T*365)} days)")
    print(f"  Bid/Ask:        ${contract['bid']:.2f} / ${contract['ask']:.2f}")

    # run MC pricing
    print(f"\nRunning Monte Carlo with {num_sims:,} simulations...")
    if option_type == "call":
        mc_result = mc.call_price_full_antithetic(S0, strike, r, sigma, T, num_sims)
        bs_price = mc.bs_call_price(S0, strike, r, sigma, T)
    else:
        mc_result = mc.put_price_full_antithetic(S0, strike, r, sigma, T, num_sims)
        bs_price = mc.bs_put_price(S0, strike, r, sigma, T)

    mc_price = mc_result.price

    # valuation verdict
    diff_pct = (mc_price - market_price) / market_price * 100 if market_price > 0 else 0

    print(f"\n--- PRICING RESULTS ---")
    print(f"  MC Fair Value:  ${mc_price:.4f}")
    print(f"  BS Fair Value:  ${bs_price:.4f}")
    print(f"  Market Price:   ${market_price:.4f}")
    print(f"  95% CI:         [${mc_result.ci_lower:.4f}, ${mc_result.ci_upper:.4f}]")
    print(f"  Std Error:      ${mc_result.std_error:.6f}")
    print(f"  Delta:          {mc_result.delta:.4f}")

    print(f"\n--- VALUATION ---")
    print(f"  MC - Market:    ${mc_price - market_price:+.4f} ({diff_pct:+.1f}%)")
    if diff_pct > 5:
        print(f"  Verdict:        UNDERVALUED — MC suggests contract is cheap")
    elif diff_pct < -5:
        print(f"  Verdict:        OVERVALUED — MC suggests contract is expensive")
    else:
        print(f"  Verdict:        FAIRLY PRICED — within noise of fair value")

    # trade stats
    mu = estimate_historical_mu(ticker)
    print(f"\n--- TRADE EVALUATION (historical drift: {mu*100:.1f}%) ---")
    trade = mc.trade_stats(S0, strike, r, sigma, T, mu, market_price, option_type, num_sims)
    print(f"  Expected PnL:   ${trade.expected_pnl:.4f}")
    print(f"  Prob Profit:    {trade.prob_profit*100:.1f}%")
    print(f"  Prob ITM:       {trade.prob_itm*100:.1f}%")
    print(f"  Prob Breakeven: {trade.prob_breakeven*100:.1f}%")

    # risk metrics
    risk = compute_risk_metrics(trade.pnl_paths, premium=market_price, rf_rate=r, T=T)
    print(f"\n--- RISK METRICS ---")
    print(f"  Sharpe Ratio:   {risk['sharpe']:.4f}")
    print(f"  VaR (5%):       ${risk['VaR_5%']:.4f}")
    print(f"  CVaR (5%):      ${risk['CVaR_5%']:.4f}")
    print(f"  Skewness:       {risk['skew']:.4f}")
    print(f"  Kurtosis:       {risk['kurtosis']:.4f}")

    # visualization
    print(f"\nGenerating visualizations with {num_viz_paths} paths...")

    strike_label = f"{ticker.upper()} ${strike} {option_type.upper()} — Exp: {expiration}"

    fig1 = plot_simulation_paths(S0, r, sigma, T, strike, strike_label,
                                  num_paths=num_viz_paths, steps=252)
    fig2 = plot_valuation_summary(
        ticker, option_type, strike, expiration,
        market_price, mc_price, bs_price,
        mc_result, contract, S0, sigma, r, T,
    )

    plt.show()

    return mc_result, bs_price, contract


def interactive_mode():
    print("\n" + "="*60)
    print("  MONTE CARLO OPTIONS ANALYZER")
    print("  Interactive Mode")
    print("="*60)

    ticker = input("\nEnter ticker symbol (e.g. PLTR, AAPL, TSLA): ").strip().upper()
    if not ticker:
        print("No ticker entered, exiting.")
        return

    print(f"\nFetching option expirations for {ticker}...")
    expirations = get_expirations(ticker)
    if not expirations:
        print(f"No options available for {ticker}")
        return

    print(f"\nAvailable expirations:")
    expiration = select_from_list(expirations, "Select expiration")

    option_type = input("\nOption type (call/put) [call]: ").strip().lower() or "call"
    if option_type not in ("call", "put"):
        print("Invalid option type, defaulting to call")
        option_type = "call"

    S0 = get_stock_price(ticker)
    calls, puts = get_option_chain(ticker, expiration)
    chain = calls if option_type == "call" else puts

    strikes = display_strikes(chain, S0)
    strike = strikes[int(input("\nSelect strike: "))]

    num_sims_input = input("\nNumber of simulations [500000]: ").strip()
    num_sims = int(num_sims_input) if num_sims_input else 500_000

    num_paths_input = input("Number of visualization paths [200]: ").strip()
    num_paths = int(num_paths_input) if num_paths_input else 200

    run_analysis(ticker, expiration, strike, option_type, num_sims, num_paths)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Monte Carlo Options Analyzer")
    parser.add_argument("--ticker", "-t", type=str, help="Ticker symbol (e.g. PLTR)")
    parser.add_argument("--expiration", "-e", type=str, help="Expiration date (YYYY-MM-DD)")
    parser.add_argument("--strike", "-s", type=float, help="Strike price")
    parser.add_argument("--type", "-T", dest="option_type", default="call",
                        choices=["call", "put"], help="Option type")
    parser.add_argument("--sims", "-n", type=int, default=500_000,
                        help="Number of MC simulations")
    parser.add_argument("--paths", "-p", type=int, default=200,
                        help="Number of visualization paths")

    args = parser.parse_args()

    if args.ticker and args.expiration and args.strike:
        run_analysis(args.ticker, args.expiration, args.strike,
                     args.option_type, args.sims, args.paths)
    else:
        interactive_mode()
