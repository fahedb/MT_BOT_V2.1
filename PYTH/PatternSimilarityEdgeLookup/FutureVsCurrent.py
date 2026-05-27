import numpy as np


# =========================
# DISTANCE
# =========================
def calc_distance(a, b):
    return np.mean(np.abs(a - b))


# =========================
# NORMALIZATION (חשוב!)
# =========================
def normalize(v):
    std = np.std(v)
    if std == 0:
        return v * 0
    return (v - np.mean(v)) / std


# =========================
# ALIGNMENT SCORE
# =========================
def calc_alignment(current_future, history_futures):
    """
    כמה current דומה ל-history
    """

    if current_future is None or len(history_futures) == 0:
        return None

    current_n = normalize(current_future)

    distances = []

    for f in history_futures:
        f_n = normalize(f)
        d = calc_distance(current_n, f_n)
        distances.append(d)

    return np.mean(distances)


# =========================
# DIRECTION MATCH
# =========================
def calc_direction_match(current_stats, history_stats_list):
    """
    כמה direction של history מתאים ל-current
    """

    curr_dir = current_stats.get("Direction")

    if curr_dir is None:
        return None

    matches = 0
    total = 0

    for s in history_stats_list:
        d = s.get("Direction")

        if d is None:
            continue

        total += 1

        if d == curr_dir:
            matches += 1

    if total == 0:
        return None

    return matches / total


# =========================
# RETURN ALIGNMENT
# =========================
def calc_return_alignment(current_stats, history_stats_list):
    """
    כמה ה-return של current קרוב לממוצע של history
    """

    curr_ret = current_stats.get("Return")

    if curr_ret is None:
        return None

    hist_returns = [
        s["Return"] for s in history_stats_list
        if s["Return"] is not None
    ]

    if len(hist_returns) == 0:
        return None

    avg_hist = np.mean(hist_returns)

    return abs(curr_ret - avg_hist)


# =========================
# MAIN FUNCTION
# =========================
def validate_current_vs_history(
    current_future,
    current_stats,
    history_futures,
    history_stats_list
):
    """
    Final validation step
    """

    if current_future is None:
        return None

    alignment = calc_alignment(current_future, history_futures)
    direction_match = calc_direction_match(current_stats, history_stats_list)
    return_alignment = calc_return_alignment(current_stats, history_stats_list)

    return {
        "AlignmentScore": alignment,
        "DirectionMatch": direction_match,
        "ReturnDiff": return_alignment
    }


# =========================
# DEBUG
# =========================
if __name__ == "__main__":

    curr = np.array([100, 102, 105, 107])
    h1 = np.array([100, 101, 104, 106])
    h2 = np.array([100, 103, 106, 108])

    history_futures = [h1, h2]

    current_stats = {
        "Return": 0.07,
        "Direction": 1
    }

    history_stats = [
        {"Return": 0.06, "Direction": 1},
        {"Return": 0.08, "Direction": 1},
    ]

    result = validate_current_vs_history(
        curr,
        current_stats,
        history_futures,
        history_stats
    )

    print("\nValidation result:")
    print(result)