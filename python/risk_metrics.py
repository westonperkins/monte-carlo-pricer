import numpy as np


def compute_risk_metrics(pnl_paths, premium=0.0, rf_rate=0.0, T=1.0):
    pnl = np.array(pnl_paths)

    mean = pnl.mean()
    std = pnl.std()

    # Annualized Sharpe ratio: excess return over risk-free, scaled to annual
    # The opportunity cost is what the premium would have earned risk-free
    rf_opportunity = premium * (np.exp(rf_rate * T) - 1.0)
    excess = mean - rf_opportunity
    sharpe = excess / std * np.sqrt(1.0 / T) if std > 0 else 0.0

    var_5 = np.percentile(pnl, 5)
    cvar_5 = pnl[pnl <= var_5].mean() if np.any(pnl <= var_5) else var_5

    skew = ((pnl - mean)**3).mean() / (std**3) if std > 0 else 0.0
    kurtosis = ((pnl - mean)**4).mean() / (std**4) if std > 0 else 0.0

    return {
        "mean": mean,
        "std": std,
        "sharpe": sharpe,
        "VaR_5%": var_5,
        "CVaR_5%": cvar_5,
        "skew": skew,
        "kurtosis": kurtosis
    }
