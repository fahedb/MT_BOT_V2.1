import numpy as np


# =========================
# DISTANCE BETWEEN FUTURES
# =========================
def calc_distance(a, b):
    """
    Distance between two future vectors
    """
    return np.mean(np.abs(a - b))


# =========================
# CONSISTENCY
# =========================
def calc_consistency(futures):
    """
    futures: list of np.array

    return:
        average pairwise distance
    """

    n = len(futures)

    if n < 2:
        return None

    distances = []

    for i in range(n):
        for j in range(i + 1, n):
            d = calc_distance(futures[i], futures[j])
            distances.append(d)

    return np.mean(distances)


# =========================
# DIRECTION AGREEMENT
# =========================
def calc_direction_agreement(stats_list):
    """
    stats_list: list of dicts with Direction

    return:
        % of majority direction
    """

    directions = [s["Direction"] for s in stats_list if s["Direction"] is not None]

    if len(directions) == 0:
        return None

    up = directions.count(1)
    down = directions.count(-1)

    total = len(directions)

    majority = max(up, down)

    return majority / total


# =========================
# AVERAGE RETURN
# =========================
def calc_avg_return(stats_list):

    returns = [s["Return"] for s in stats_list if s["Return"] is not None]

    if len(returns) == 0:
        return None

    return np.mean(returns)


# =========================
# MAIN FUNCTION
# =========================
def analyze_future_cluster(futures, stats_list):
    """
    futures: list of np.array
    stats_list: list of stats dicts

    return:
        dict with cluster metrics
    """

    if len(futures) == 0:
        return None

    consistency = calc_consistency(futures)
    direction_agreement = calc_direction_agreement(stats_list)
    avg_return = calc_avg_return(stats_list)

    return {
        "NumSamples": len(futures),
        "Consistency": consistency,
        "DirectionAgreement": direction_agreement,
        "AvgReturn": avg_return
    }


# =========================
# DEBUG
# =========================
if __name__ == "__main__":

    f1 = np.array([100, 102, 104, 105])
    f2 = np.array([100, 101, 103, 104])
    f3 = np.array([100, 99, 98, 97])

    futures = [f1, f2, f3]

    stats_list = [
        {"Return": 0.05, "Direction": 1},
        {"Return": 0.04, "Direction": 1},
        {"Return": -0.03, "Direction": -1},
    ]

    result = analyze_future_cluster(futures, stats_list)

    print("\nCluster Analysis:")
    print(result)