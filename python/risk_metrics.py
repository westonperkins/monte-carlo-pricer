import numpy as np

def compute_risk_metrics(pnl_paths):
    pnl = np.array(pnl_paths)

    mean = pnl.mean()
    std = pnl.std()
    sharpe = mean / std if std > 0 else 0.0

    var_5 = np.percentile(pnl, 5)
    cvar_5 = pnl[pnl <= var_5].mean()

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