import sys
import os
import yfinance as yf
from datetime import datetime
import numpy as np

# Allow import of C++ module
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build")))
import mc_pricer_py as mc
from market_data import estimate_historical_mu


def evaluate_option(ticker, expiration, strike, option_type="call", mu=None, N=300000):
    stock = yf.Ticker(ticker)

    # Current stock price
    S0 = stock.history(period="1d")["Close"].iloc[-1]

    # Risk-free rate from 10-year Treasury
    tnx = yf.Ticker("^TNX")
    tnx_hist = tnx.history(period="1d")
    r = float(tnx_hist["Close"].iloc[-1]) / 100.0 if not tnx_hist.empty else 0.045

    # Fetch option chain
    opt_chain = stock.option_chain(expiration)
    chain = opt_chain.calls if option_type == "call" else opt_chain.puts

    # Find the correct strike
    option_row = chain[chain["strike"] == strike]

    if option_row.empty:
        raise ValueError(f"Strike {strike} not found.")

    premium = float(option_row["lastPrice"].iloc[0])
    implied_vol = float(option_row["impliedVolatility"].iloc[0])

    # Time to expiration (trading days / 252)
    exp_date = datetime.strptime(expiration, "%Y-%m-%d")
    today = datetime.today()
    trading_days = int(np.busday_count(today.date(), exp_date.date()))
    T = trading_days / 252.0

    # Estimate drift from historical returns if not provided
    if mu is None:
        mu = estimate_historical_mu(ticker)

    print("\n--- Market Data ---")
    print(f"Stock Price: ${S0:.2f}")
    print(f"Premium:     ${premium:.2f}")
    print(f"Implied Vol: {implied_vol*100:.1f}%")
    print(f"Risk-Free:   {r*100:.2f}%")
    print(f"Time (years): {T:.4f} ({trading_days} trading days)")
    print(f"Drift (mu):  {mu*100:.1f}%")
    print(f"Type:        {option_type.upper()}")

    # Run Monte Carlo trade evaluation
    res = mc.trade_stats(S0, strike, r, implied_vol, T, mu, premium, option_type, N)

    print("\n--- Monte Carlo Trade Stats ---")
    print(f"Expected PnL:   ${res.expected_pnl:.4f}")
    print(f"Prob Profit:    {res.prob_profit*100:.1f}%")
    print(f"Prob ITM:       {res.prob_itm*100:.1f}%")
    print(f"Prob Breakeven: {res.prob_breakeven*100:.1f}%")


# Example usage
if __name__ == "__main__":
    evaluate_option(
        ticker="PLTR",
        expiration="2026-06-12",
        strike=205,
        option_type="call",
    )
