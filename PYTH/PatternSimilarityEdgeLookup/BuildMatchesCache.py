from HistoryLoader import load_history as load_history_data
from CurrentLoader import load_history as load_current_data

from DataVectorBuilder import build_all_vectors
from VectorSearchEngine import search_all_fast

import json


def run():

    print("\n=== BUILD MATCHES CACHE ===")

    candles_hist, indicators_hist = load_history_data()
    candles_curr, indicators_curr = load_current_data()

    vectors_hist = build_all_vectors(candles_hist, indicators_hist)
    vectors_curr = build_all_vectors(candles_curr, indicators_curr)

    matches = search_all_fast(vectors_curr, vectors_hist)

    # =========================
    # SAVE TO JSON
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


if __name__ == "__main__":
    run()