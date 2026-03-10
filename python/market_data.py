import yfinance as yf
from datetime import datetime


def get_stock_price(ticker):
    stock = yf.Ticker(ticker)
    hist = stock.history(period="1d")
    if hist.empty:
        raise ValueError(f"Could not fetch price for {ticker}")
    return float(hist["Close"].iloc[-1])


def get_option_chain(ticker, expiration):
    stock = yf.Ticker(ticker)
    chain = stock.option_chain(expiration)
    return chain.calls, chain.puts


def get_expirations(ticker):
    stock = yf.Ticker(ticker)
    return list(stock.options)


def get_risk_free_rate():
    try:
        tnx = yf.Ticker("^TNX")
        hist = tnx.history(period="1d")
        if not hist.empty:
            return float(hist["Close"].iloc[-1]) / 100.0
    except Exception:
        pass
    return 0.045


def time_to_expiry(expiration_str):
    exp_date = datetime.strptime(expiration_str, "%Y-%m-%d")
    today = datetime.today()
    days = (exp_date - today).days
    if days <= 0:
        raise ValueError(f"Expiration {expiration_str} is in the past")
    return days / 365.0


def fetch_contract(ticker, expiration, strike, option_type="call"):
    calls, puts = get_option_chain(ticker, expiration)
    chain = calls if option_type == "call" else puts

    row = chain[chain["strike"] == strike]
    if row.empty:
        available = sorted(chain["strike"].tolist())
        raise ValueError(
            f"Strike {strike} not found. Available strikes:\n{available}"
        )

    row = row.iloc[0]
    return {
        "strike": float(row["strike"]),
        "lastPrice": float(row["lastPrice"]),
        "bid": float(row["bid"]),
        "ask": float(row["ask"]),
        "impliedVolatility": float(row["impliedVolatility"]),
        "volume": int(row["volume"]) if row["volume"] == row["volume"] else 0,
        "openInterest": int(row["openInterest"]) if row["openInterest"] == row["openInterest"] else 0,
        "inTheMoney": bool(row["inTheMoney"]),
    }
