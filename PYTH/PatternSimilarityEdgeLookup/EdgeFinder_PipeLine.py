import json
import pandas as pd
import numpy as np

from HistoryLoader import load_history as load_history_data
from CurrentLoader import load_history as load_current_data

from FutureExtractor import get_future
from CalcFutureReturn import compute_future_stats


# =========================
# LOAD MATCHES
# =========================
def load_matches(path="matches_cache.json"):
    with open(path, "r") as f:
        data = json.load(f)
    return data["data"] if "data" in data else data


# =========================
# BUILD INDEX MAP
# =========================
def build_index_map(candles):
    timepoints = sorted(candles.keys())
    return {tp: i for i, tp in enumerate(timepoints)}


# =========================
# BUILD DATASET (FIXED)
# =========================
def build_dataset():

    print("\n=== BUILD DATASET ===")

    candles_hist, _ = load_history_data()
    candles_curr, _ = load_current_data()

    hist_map = build_index_map(candles_hist)
    curr_map = build_index_map(candles_curr)

    df_hist = pd.DataFrame.from_dict(candles_hist, orient="index")
    df_curr = pd.DataFrame.from_dict(candles_curr, orient="index")

    matches = load_matches()

    rows = []

    for i, (curr_tp_str, match_list) in enumerate(matches.items()):

        curr_tp = int(curr_tp_str)

        # CURRENT FUTURE
        curr_future = get_future(curr_tp, df_curr, curr_map)
        curr_stats = compute_future_stats(curr_future)

        if curr_stats is None:
            continue

        curr_return = curr_stats["Return"]

        for m in match_list:

            match_tp = m["MatchTP"]

            hist_future = get_future(match_tp, df_hist, hist_map)
            hist_stats = compute_future_stats(hist_future)

            if hist_stats is None:
                continue

            rows.append({
                "Distance": m["Distance"],
                "Rank": m["Rank"],
                "HistReturn": hist_stats["Return"],
                "CurrReturn": curr_return
            })

        if i % 200 == 0:
            print(f"Processed {i}")

    df = pd.DataFrame(rows)

    print(f"\nDataset built: {len(df)} rows")

    return df


# =========================
# EDGE FINDER (FIXED)
# =========================
def find_edge(df):

    print("\n=== EDGE FINDER ===")

    # 🔥 הכי חשוב — CURRENT
    baseline = df["CurrReturn"].mean()
    print(f"Baseline Current Return: {baseline:.6f}")

    results = []

    # =========================
    # DISTANCE THRESHOLDS
    # =========================
    distances = np.linspace(df["Distance"].min(), df["Distance"].max(), 30)

    for d in distances:

        subset = df[df["Distance"] <= d]

        if len(subset) < 1000:
            continue

        avg_ret = subset["CurrReturn"].mean()
        winrate = (subset["CurrReturn"] > 0).mean()

        edge = avg_ret - baseline

        results.append({
            "Type": "Distance",
            "Threshold": d,
            "Samples": len(subset),
            "AvgReturn": avg_ret,
            "WinRate": winrate,
            "Edge": edge
        })

    # =========================
    # RANK THRESHOLDS
    # =========================
    for r in range(1, 21):

        subset = df[df["Rank"] <= r]

        if len(subset) < 1000:
            continue

        avg_ret = subset["CurrReturn"].mean()
        winrate = (subset["CurrReturn"] > 0).mean()

        edge = avg_ret - baseline

        results.append({
            "Type": "Rank",
            "Threshold": r,
            "Samples": len(subset),
            "AvgReturn": avg_ret,
            "WinRate": winrate,
            "Edge": edge
        })

    results_df = pd.DataFrame(results)

    print("\n=== TOP EDGE SIGNALS ===")
    print(results_df.sort_values("Edge", ascending=False).head(10))

    return results_df


# =========================
# DEBUG ANALYSIS
# =========================
def debug_checks(df):

    print("\n=== DEBUG ===")

    # קורלציה אמיתית
    corr = df[["Distance", "CurrReturn"]].corr()
    print("\nDistance vs Current Return correlation:")
    print(corr)

    print("\nHist vs Curr correlation:")
    print(df[["HistReturn", "CurrReturn"]].corr())


# =========================
# MAIN
# =========================
def run():

    df = build_dataset()

    debug_checks(df)

    results = find_edge(df)

    return results


if __name__ == "__main__":
    run()