import numpy as np


# =========================
# NORMALIZATION
# =========================
def normalize(x):
    mean = np.mean(x)
    std = np.std(x)

    if std == 0:
        return x * 0

    return (x - mean) / std


# =========================
# TREND (direction)
# =========================
def calc_trend(x):
    return x[-1] - x[0]


# =========================
# DISTANCE
# =========================
def calc_distance(a, b):

    # shape distance
    shape = np.mean(np.abs(a - b))

    # trend penalty
    trend_a = calc_trend(a)
    trend_b = calc_trend(b)

    trend_diff = abs(trend_a - trend_b)

    # combine
    return shape + 0.3 * trend_diff


# =========================
# MAIN SIMILARITY
# =========================
def compute_similarity(patterns):

    print("\n=== Computing Similarity ===")

    results = []

    for i, p in enumerate(patterns):

        curr = p["CurrPattern"]
        hist = p["HistPattern"]

        # normalize
        curr_n = normalize(curr)
        hist_n = normalize(hist)

        distance = calc_distance(curr_n, hist_n)

        results.append({
            "CurrentTP": p["CurrentTP"],
            "MatchTP": p["MatchTP"],
            "Score": p["Score"],   # שלך
            "Distance": distance
        })

        if i % 50000 == 0:
            print(f"Processed {i}")

    print(f"Similarity computed: {len(results)}")

    return results