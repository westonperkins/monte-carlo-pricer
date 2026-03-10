import yfinance as yf
from datetime import datetime
import time
import functools

# cache with TTL — stores (timestamp, value) pairs
_cache = {}
CACHE_TTL = 300  # 5 minutes


def _cached(key, fn):
    now = time.time()
    if key in _cache:
        ts, val = _cache[key]
        if now - ts < CACHE_TTL:
            return val
    val = fn()
    _cache[key] = (now, val)
    return val


def _retry(fn, retries=3, delay=2):
    for attempt in range(retries):
        try:
            return fn()
        except Exception as e:
            if attempt == retries - 1:
                raise
            err = str(e).lower()
            if "rate" in err or "too many" in err or "429" in err:
                time.sleep(delay * (attempt + 1))
            else:
                raise


def get_stock_price(ticker):
    def fetch():
        stock = yf.Ticker(ticker)
        hist = stock.history(period="1d")
        if hist.empty:
            raise ValueError(f"Could not fetch price for {ticker}")
        return float(hist["Close"].iloc[-1])
    return _cached(f"price:{ticker}", lambda: _retry(fetch))


def get_option_chain(ticker, expiration):
    def fetch():
        stock = yf.Ticker(ticker)
        chain = stock.option_chain(expiration)
        return chain.calls, chain.puts
    return _cached(f"chain:{ticker}:{expiration}", lambda: _retry(fetch))


def get_expirations(ticker):
    def fetch():
        stock = yf.Ticker(ticker)
        return list(stock.options)
    return _cached(f"exp:{ticker}", lambda: _retry(fetch))


def get_risk_free_rate():
    def fetch():
        tnx = yf.Ticker("^TNX")
        hist = tnx.history(period="1d")
        if not hist.empty:
            return float(hist["Close"].iloc[-1]) / 100.0
        return 0.045
    try:
        return _cached("rfr", lambda: _retry(fetch))
    except Exception:
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
