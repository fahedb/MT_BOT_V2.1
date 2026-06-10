import numpy as np


# =========================
# RETURN
# =========================
def calc_return(future):
    """
    Calculate total return over future window
    """

    if future is None or len(future) == 0:
        return None

    start = future[0]
    end = future[-1]

    # הגנה מחלוקה ב-0
    if start == 0:
        return None

    return (end - start) / start


# =========================
# MAX MOVE
# =========================
def calc_max_move(future):
    """
    Calculate max up and max down relative to start
    """

    if future is None or len(future) == 0:
        return None, None

    start = future[0]

    if start == 0:
        return None, None

    max_price = np.max(future)
    min_price = np.min(future)

    max_up = (max_price - start) / start
    max_down = (min_price - start) / start

    return max_up, max_down


# =========================
# DIRECTION
# =========================
def calc_direction(future):

    max_up, max_down = calc_max_move(future)

    if max_up is None:
        return None

    if max_up > abs(max_down):
        return 1

    elif abs(max_down) > max_up:
        return -1

    return 0

# =========================
# FULL CALCULATION
# =========================
def compute_future_stats(future):
    """
    Convert future vector → stats dict
    """

    if future is None:
        return None

    ret = calc_return(future)
    max_up, max_down = calc_max_move(future)
    direction = calc_direction(future)

    return {
        "Return": ret,
        "MaxUp": max_up,
        "MaxDown": max_down,
        "Direction": direction
    }


# =========================
# BATCH VERSION
# =========================
def compute_futures_stats(futures_dict):
    """
    futures_dict: {tp: future_vector}

    returns:
    {tp: stats}
    """

    results = {}

    for i, (tp, future) in enumerate(futures_dict.items()):

        stats = compute_future_stats(future)

        if stats is not None:
            results[tp] = stats

        if i % 10000 == 0:
            print(f"Processed {i}")

    print(f"Computed stats for: {len(results)} futures")

    return results


# =========================
# DEBUG
# =========================
if __name__ == "__main__":

    sample = np.array([100, 102, 105, 103, 108])

    stats = compute_future_stats(sample)

    print("\nSample stats:")
    print(stats)