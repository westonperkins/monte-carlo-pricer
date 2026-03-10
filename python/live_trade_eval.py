import sys
import os
import yfinance as yf
from datetime import datetime

# Allow import of C++ module
sys.path.append(os.path.abspath("../build"))
import mc_pricer_py as mc


def evaluate_call(ticker, expiration, strike, mu, N=300000):
    stock = yf.Ticker(ticker)

    # Current stock price
    S0 = stock.history(period="1d")["Close"].iloc[-1]

    # Risk-free rate assumption (simplified)
    r = 0.05

    # Fetch option chain
    opt_chain = stock.option_chain(expiration)
    calls = opt_chain.calls

    # Find the correct strike
    option_row = calls[calls["strike"] == strike]

    if option_row.empty:
        raise ValueError("Strike not found.")

    premium = float(option_row["lastPrice"].iloc[0])
    implied_vol = float(option_row["impliedVolatility"].iloc[0])

    # Time to expiration
    exp_date = datetime.strptime(expiration, "%Y-%m-%d")
    T = (exp_date - datetime.today()).days / 365.0

    print("\n--- Market Data ---")
    print("Stock Price:", S0)
    print("Premium:", premium)
    print("Implied Vol:", implied_vol)
    print("Time (years):", T)

    # Run Monte Carlo trade evaluation
    res = mc.trade_stats(S0, strike, r, implied_vol, T, mu, premium, N)

    print("\n--- Monte Carlo Trade Stats ---")
    print("Expected PnL:", res.expected_pnl)
    print("Prob Profit:", res.prob_profit)
    print("Prob ITM:", res.prob_itm)
    print("Prob Breakeven:", res.prob_breakeven)


# Example usage
if __name__ == "__main__":
    evaluate_call(
        ticker="PLTR",
        expiration="2026-06-12",
        strike=205,
        mu=0.08  # your belief about annual return
    )