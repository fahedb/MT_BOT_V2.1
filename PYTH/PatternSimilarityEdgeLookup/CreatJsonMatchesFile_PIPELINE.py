from HistoryLoader import load_history as load_history_data
from CurrentLoader import load_history as load_current_data

from DataVectorBuilder import build_all_vectors
from VectorSearchEngine import search_all_fast

from FutureExtractor import get_future
from CalcFutureReturn import compute_future_stats
from FutureSimilarity import analyze_future_cluster
from FutureVsCurrent import validate_current_vs_history

import pandas as pd
import json

# =========================
# HELPER: build index map
# =========================
def build_index_map(candles):
    timepoints = sorted(candles.keys())
    return {tp: i for i, tp in enumerate(timepoints)}, timepoints


# =========================
# MAIN PIPELINE
# =========================

def run_pipeline():

    print("\n==============================")
    print("🚀 START PIPELINE")
    print("==============================")

    # =========================
    # 1. LOAD DATA
    # =========================
    candles_hist, indicators_hist = load_history_data()
    candles_curr, indicators_curr = load_current_data()

    # =========================
    # 2. BUILD VECTORS
    # =========================
    vectors_hist = build_all_vectors(candles_hist, indicators_hist)
    vectors_curr = build_all_vectors(candles_curr, indicators_curr)

    # =========================
    # 3. SEARCH MATCHES
    # =========================
    matches = search_all_fast(vectors_curr, vectors_hist)

    print(f"\nTotal matches: {len(matches)}")

    # =========================
    # SAVE JSON 🔥
    # =========================
    output = {}

    for row in matches:
        curr = int(row["CurrentTP"])

        output.setdefault(curr, []).append({
            "MatchTP": int(row["MatchTP"]),
            "Distance": float(row["Distance"]),
            "Rank": int(row["Rank"])
        })

    with open("matches_cache.json", "w") as f:
        json.dump(output, f)

    print("✅ Saved matches_cache.json")

    # ❗❗ עצור כאן
    return

# =========================
# ENTRY POINT
# =========================
if __name__ == "__main__":
    run_pipeline()