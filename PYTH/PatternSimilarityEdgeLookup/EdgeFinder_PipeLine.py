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
def load_matches(path="matches_cache_v2.json"):
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

    print("\n=== BUILD DATASET BY CurrTP ===")

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

        curr_future = get_future(curr_tp, df_curr, curr_map)
        curr_stats = compute_future_stats(curr_future)

        if curr_stats is None:
            continue

        hist_returns = []
        distances = []

        for m in match_list:

            match_tp = int(m["MatchTP"])
            distance = float(m["Distance"])

            hist_future = get_future(match_tp, df_hist, hist_map)
            hist_stats = compute_future_stats(hist_future)

            if hist_stats is None:
                continue

            hist_returns.append(float(hist_stats["Return"]))
            distances.append(distance)

        if len(hist_returns) == 0:
            continue

        weights = 1.0 / (np.array(distances) + 1e-9)

        pred_return = np.average(hist_returns, weights=weights)

        rows.append({
            "CurrTP": curr_tp,
            "CurrReturn": float(curr_stats["Return"]),
            "PredReturn": float(pred_return),
            "AvgHistReturn": float(np.mean(hist_returns)),
            "MedianHistReturn": float(np.median(hist_returns)),
            "MinDistance": float(np.min(distances)),
            "AvgDistance": float(np.mean(distances)),
            "MatchesCount": len(hist_returns),
        })

        if i % 200 == 0:
            print(f"Processed {i}")

    df = pd.DataFrame(rows)

    print(f"\nDataset built: {len(df)} current patterns")

    return df


# =========================
# EDGE FINDER (FIXED)
# =========================
def find_edge(df):

    print("\n=== EDGE FINDER BY CONFIDENCE ===")

    baseline = df["CurrReturn"].mean()
    print(f"Baseline Current Return: {baseline:.8f}")

    df = df.copy()
    df["PredAbs"] = df["PredReturn"].abs()

    results = []

    for q in [0.50, 0.60, 0.70, 0.80, 0.90, 0.95]:

        threshold = df["PredAbs"].quantile(q)
        subset = df[df["PredAbs"] >= threshold]

        if len(subset) < 100:
            continue

        acc = ((subset["PredReturn"] > 0) == (subset["CurrReturn"] > 0)).mean()
        avg_ret = subset["CurrReturn"].mean()

        results.append({
            "TopConfidence": f"Top {int((1-q)*100)}%",
            "Threshold": threshold,
            "Samples": len(subset),
            "DirectionAccuracy": acc,
            "AvgCurrReturn": avg_ret,
            "Edge": avg_ret - baseline
        })

    results_df = pd.DataFrame(results)

    print("\n=== TOP CONFIDENCE RESULTS ===")
    print(results_df)

    return results_df
# =========================
# DEBUG ANALYSIS
# =========================
def debug_checks(df):

    print("\n=== DEBUG BY CurrTP ===")

    print("\nPrediction correlation:")
    print(df[["PredReturn", "AvgHistReturn", "MedianHistReturn", "CurrReturn"]].corr())

    df["PredUp"] = df["PredReturn"] > 0
    df["RealUp"] = df["CurrReturn"] > 0
    df["CorrectDirection"] = df["PredUp"] == df["RealUp"]

    print("\nDirection Accuracy:")
    print(df["CorrectDirection"].mean())

    print("\nAvg Return when PredUp:")
    print(df[df["PredUp"]]["CurrReturn"].mean())

    print("\nAvg Return when PredDown:")
    print(df[~df["PredUp"]]["CurrReturn"].mean())

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