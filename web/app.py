#!/usr/bin/env python3
"""Flask web server for Monte Carlo Options Analyzer."""

import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build")))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "python")))

from flask import Flask, render_template, jsonify, request
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
from risk_metrics import compute_risk_metrics
import numpy as np
import traceback

app = Flask(__name__)


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/expirations/<ticker>")
def api_expirations(ticker):
    try:
        price = get_stock_price(ticker)
        expirations = get_expirations(ticker)
        return jsonify({"price": price, "expirations": expirations})
    except Exception as e:
        return jsonify({"error": str(e)}), 400


@app.route("/api/chain/<ticker>/<expiration>")
def api_chain(ticker, expiration):
    try:
        calls, puts = get_option_chain(ticker, expiration)
        S0 = get_stock_price(ticker)

        def chain_to_list(df):
            rows = []
            for _, row in df.iterrows():
                rows.append({
                    "strike": float(row["strike"]),
                    "lastPrice": float(row["lastPrice"]),
                    "bid": float(row["bid"]),
                    "ask": float(row["ask"]),
                    "impliedVolatility": float(row["impliedVolatility"]),
                    "volume": int(row["volume"]) if row["volume"] == row["volume"] else 0,
                    "openInterest": int(row["openInterest"]) if row["openInterest"] == row["openInterest"] else 0,
                    "inTheMoney": bool(row["inTheMoney"]),
                })
            return rows

        return jsonify({
            "spot": S0,
            "calls": chain_to_list(calls),
            "puts": chain_to_list(puts),
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 400


@app.route("/api/analyze", methods=["POST"])
def api_analyze():
    try:
        data = request.json
        ticker = data["ticker"]
        expiration = data["expiration"]
        strike = float(data["strike"])
        option_type = data.get("optionType", "call")
        num_sims = int(data.get("numSims", 500_000))
        num_paths = int(data.get("numPaths", 200))
        steps = int(data.get("steps", 252))

        S0 = get_stock_price(ticker)
        r = get_risk_free_rate()
        T = time_to_expiry(expiration)
        contract = fetch_contract(ticker, expiration, strike, option_type)

        market_price = contract["lastPrice"]
        sigma = contract["impliedVolatility"]

        # MC pricing
        if option_type == "call":
            mc_result = mc.call_price_full_antithetic(S0, strike, r, sigma, T, num_sims)
            bs_price = mc.bs_call_price(S0, strike, r, sigma, T)
        else:
            mc_result = mc.put_price_full_antithetic(S0, strike, r, sigma, T, num_sims)
            bs_price = mc.bs_put_price(S0, strike, r, sigma, T)

        mc_price = mc_result.price

        # trade stats
        mu = float(data.get("mu", 0)) or estimate_historical_mu(ticker)
        trade = mc.trade_stats(S0, strike, r, sigma, T, mu, market_price, option_type, num_sims)
        risk = compute_risk_metrics(trade.pnl_paths, premium=market_price, rf_rate=r, T=T)

        # simulate paths for visualization
        flat = mc.simulate_paths(S0, r, sigma, T, num_paths, steps)
        paths = np.array(flat).reshape(num_paths, steps + 1)
        t_axis = np.linspace(0, T, steps + 1).tolist()

        # downsample paths for JSON transfer (every 2nd step for 252 steps)
        step_size = max(1, steps // 126)
        indices = list(range(0, steps + 1, step_size))
        if indices[-1] != steps:
            indices.append(steps)
        sampled_t = [t_axis[i] for i in indices]
        sampled_paths = paths[:, indices].tolist()

        terminal = paths[:, -1]
        itm_mask = (terminal > strike) if option_type == "call" else (terminal < strike)

        diff_pct = (mc_price - market_price) / market_price * 100 if market_price > 0 else 0

        return jsonify({
            "spot": S0,
            "riskFreeRate": r,
            "timeToExpiry": T,
            "sigma": sigma,
            "marketPrice": market_price,
            "mcPrice": mc_price,
            "bsPrice": bs_price,
            "ciLower": mc_result.ci_lower,
            "ciUpper": mc_result.ci_upper,
            "stdError": mc_result.std_error,
            "delta": mc_result.delta,
            "diffPct": diff_pct,
            "mu": mu,
            "expectedPnl": trade.expected_pnl,
            "probProfit": trade.prob_profit,
            "probItm": trade.prob_itm,
            "probBreakeven": trade.prob_breakeven,
            "risk": {
                "sharpe": risk["sharpe"],
                "var5": risk["VaR_5%"],
                "cvar5": risk["CVaR_5%"],
                "skew": risk["skew"],
                "kurtosis": risk["kurtosis"],
            },
            "contract": contract,
            "paths": {
                "t": sampled_t,
                "data": sampled_paths,
                "itm": itm_mask.tolist(),
                "terminal": terminal.tolist(),
            },
        })
    except Exception as e:
        traceback.print_exc()
        return jsonify({"error": str(e)}), 400


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5050))
    debug = os.environ.get("FLASK_DEBUG", "1") == "1"
    print(f"\n  Monte Carlo Options Analyzer")
    print(f"  Open http://localhost:{port} in your browser\n")
    app.run(host="0.0.0.0", port=port, debug=debug)
