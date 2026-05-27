import pandas as pd
import numpy as np


# =========================
# MAIN ANALYZER
# =========================
def analyze(results):

    print("\n=== ANALYZER ===")

    df = pd.DataFrame(results)

    # ניקוי
    df = df.dropna(subset=["Distance", "HistReturn"])

    print(f"Total rows: {len(df)}")

    # =========================
    # 1. BINNING לפי Distance
    # =========================
    df["DistanceBin"] = pd.qcut(df["Distance"], 10, duplicates="drop")

    grouped = df.groupby("DistanceBin")

    stats = grouped.agg({
        "HistReturn": ["mean", "count"],
        "CurrReturn": "mean"
    })

    print("\n=== Distance → Return ===")
    print(stats)

    # =========================
    # 2. CORRELATION
    # =========================
    corr = df["Distance"].corr(df["HistReturn"])

    print(f"\nCorrelation (Distance vs HistReturn): {corr:.6f}")

    # =========================
    # 3. TOP SIMILAR ONLY
    # =========================
    threshold = df["Distance"].quantile(0.1)

    best = df[df["Distance"] <= threshold]

    print("\n=== TOP 10% SIMILAR ===")
    print(f"Trades: {len(best)}")
    print(f"Avg Return: {best['HistReturn'].mean():.6f}")

    # =========================
    # 4. RANDOM BASELINE
    # =========================
    print("\n=== BASELINE ===")
    print(f"Avg Return (All): {df['HistReturn'].mean():.6f}")

    # =========================
    # 5. CURRENT VS HISTORY
    # =========================
    valid = df.dropna(subset=["CurrReturn"])

    if len(valid) > 0:
        diff = (valid["CurrReturn"] - valid["HistReturn"]).abs().mean()

        print("\n=== CURRENT vs HISTORY ===")
        print(f"Avg absolute diff: {diff:.6f}")

    print("\n=== ANALYSIS DONE ===")

    return df