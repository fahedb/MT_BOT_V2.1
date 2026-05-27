import json
import pandas as pd
import numpy as np

from HistoryLoader import load_history as load_history_data
from CurrentLoader import load_history as load_current_data

from FutureExtractor import get_future
from CalcFutureReturn import compute_future_stats


# =========================
# CONFIG
# =========================
MIN_SAMPLES = 10

DISTANCE_THRESHOLDS = [
    0.013,
    0.014,
    0.015,
    0.017,
    0.020
]

AGREEMENT_THRESHOLDS = np.arange(0.50, 1.01, 0.05)

EMA_SLOPE_THRESHOLDS = [
    0.0,
    0.00001,
    0.00003,
    0.00005,
    0.0001
]


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
# GET EMA SLOPE
# =========================
def get_h1_ema20_slope(tp, indicators):

    try:
        return indicators[tp]["EMA"]["H1"][20]["slope"]
    except:
        return None


# =========================
# CLUSTER ANALYSIS
# =========================
def analyze_cluster(history_stats):

    directions = []

    for s in history_stats:

        d = s["Direction"]

        if d == 1 or d == -1:
            directions.append(d)

    if len(directions) == 0:
        return None

    up_count = directions.count(1)
    down_count = directions.count(-1)

    total = len(directions)

    agreement = max(up_count, down_count) / total

    cluster_direction = 1 if up_count > down_count else -1

    avg_hist_return = np.mean([
        s["Return"]
        for s in history_stats
        if s["Return"] is not None
    ])

    return {
        "DirectionAgreement": agreement,
        "ClusterDirection": cluster_direction,
        "Samples": total,
        "AvgHistReturn": avg_hist_return
    }


# =========================
# BUILD SIGNALS
# =========================
def build_signals():

    print("\n==============================")
    print("🚀 BUILD SIGNALS")
    print("==============================")

    # =========================
    # LOAD DATA
    # =========================
    candles_hist, indicators_hist = load_history_data()
    candles_curr, indicators_curr = load_current_data()

    hist_map = build_index_map(candles_hist)
    curr_map = build_index_map(candles_curr)

    df_hist = pd.DataFrame.from_dict(
        candles_hist,
        orient="index"
    )

    df_curr = pd.DataFrame.from_dict(
        candles_curr,
        orient="index"
    )

    matches = load_matches()

    results = []

    # =========================
    # LOOP CURRENT
    # =========================
    for i, (curr_tp_str, match_list) in enumerate(matches.items()):

        curr_tp = int(curr_tp_str)

        # =========================
        # CURRENT FUTURE
        # =========================
        # DEBUG / RESEARCH ONLY
        # =========================
        curr_future = get_future(
            curr_tp,
            df_curr,
            curr_map
        )

        curr_stats = compute_future_stats(
            curr_future
        )

        curr_direction = None
        prediction_correct = None
        curr_return = None

        if curr_stats is not None:

            curr_direction = curr_stats["Direction"]

            curr_return = curr_stats["Return"]

        # =========================
        # EMA SLOPE
        # =========================
        curr_h1_slope = get_h1_ema20_slope(
            curr_tp,
            indicators_curr
        )

        if curr_h1_slope is None:
            continue

        # =========================
        # HISTORY FUTURES
        # =========================
        history_stats = []

        distances = []

        ranks = []

        for m in match_list:

            match_tp = m["MatchTP"]

            hist_future = get_future(
                match_tp,
                df_hist,
                hist_map
            )

            hist_stats = compute_future_stats(
                hist_future
            )

            if hist_stats is None:
                continue

            history_stats.append(hist_stats)

            distances.append(m["Distance"])

            ranks.append(m["Rank"])

        if len(history_stats) < MIN_SAMPLES:
            continue

        # =========================
        # CLUSTER ANALYSIS
        # =========================
        cluster = analyze_cluster(
            history_stats
        )

        if cluster is None:
            continue

        # =========================
        # DEBUG ONLY
        # =========================
        if curr_direction in (1, -1):

            prediction_correct = int(
                cluster["ClusterDirection"] == curr_direction
            )

        # =========================
        # BUILD SIGNAL
        # =========================
        results.append({

            "CurrentTP": curr_tp,

            "SignalDirection": cluster["ClusterDirection"],

            "DirectionAgreement": cluster["DirectionAgreement"],

            "AvgHistReturn": cluster["AvgHistReturn"],

            "AvgDistance": np.mean(distances),

            "MinDistance": np.min(distances),

            "BestRank": np.min(ranks),

            "Samples": cluster["Samples"],

            "H1_EMA20_Slope": curr_h1_slope,

            "Abs_H1_EMA20_Slope": abs(curr_h1_slope),

            # =========================
            # DEBUG / RESEARCH ONLY
            # =========================
            "CurrDirection": curr_direction,

            "PredictionCorrect": prediction_correct,

            "CurrReturn": curr_return
        })

        if i % 100 == 0:
            print(f"Processed {i}")

    # =========================
    # FINAL DF
    # =========================
    df = pd.DataFrame(results)

    print("\n==============================")
    print("✅ SIGNALS READY")
    print("==============================")

    print(df.head())

    return df


# =========================
# ANALYZE FILTERS
# =========================
def analyze_filters(df):

    print("\n==============================")
    print("📊 FILTER ANALYSIS")
    print("==============================")

    # רק שורות עם debug info
    debug_df = df[
        df["PredictionCorrect"].notna()
    ]

    baseline = debug_df["PredictionCorrect"].mean()

    print(f"\nBaseline Accuracy: {baseline:.4f}")

    rows = []

    for d in DISTANCE_THRESHOLDS:

        for a in AGREEMENT_THRESHOLDS:

            for s in EMA_SLOPE_THRESHOLDS:

                subset = debug_df[
                    (debug_df["AvgDistance"] <= d) &
                    (debug_df["DirectionAgreement"] >= a) &
                    (debug_df["Abs_H1_EMA20_Slope"] >= s)
                ]

                if len(subset) < 100:
                    continue

                accuracy = subset["PredictionCorrect"].mean()

                edge = accuracy - baseline

                avg_return = subset["CurrReturn"].mean()

                rows.append({

                    "DistanceThreshold": d,

                    "AgreementThreshold": a,

                    "SlopeThreshold": s,

                    "Samples": len(subset),

                    "Accuracy": accuracy,

                    "AvgReturn": avg_return,

                    "Edge": edge
                })

    result_df = pd.DataFrame(rows)

    print("\n==============================")
    print("🔥 TOP FILTER COMBINATIONS")
    print("==============================")

    print(
        result_df
        .sort_values(
            ["Edge", "Samples"],
            ascending=[False, False]
        )
        .head(50)
    )

    return result_df


# =========================
# MAIN
# =========================
def run():

    df = build_signals()

    result_df = analyze_filters(df)

    return df, result_df


# =========================
# ENTRY
# =========================
if __name__ == "__main__":

    run()