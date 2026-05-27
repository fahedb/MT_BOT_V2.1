import numpy as np

TLLEN = 50
VISION = 10


# =========================
# GET FUTURE WINDOW
# =========================
def get_future(df, time_to_index, tp):

    if tp not in time_to_index:
        return None

    start_idx = time_to_index[tp] + TLLEN
    end_idx = start_idx + VISION

    if end_idx >= len(df):
        return None

    return df.iloc[start_idx:end_idx]["ClosePrice"].values


# =========================
# RETURN CALCULATION
# =========================
def calc_return(future):

    if future is None or len(future) == 0:
        return None

    start = future[0]
    end = future[-1]

    return (end - start) / (start + 1e-6)


# =========================
# OPTIONAL: MAX MOVE
# =========================
def calc_max_move(future):

    if future is None or len(future) == 0:
        return None, None

    start = future[0]

    max_up = (np.max(future) - start) / (start + 1e-6)
    max_down = (np.min(future) - start) / (start + 1e-6)

    return max_up, max_down


# =========================
# MAIN ENGINE
# =========================
def compute_future(similarity_results,
                   df_history, map_history,
                   df_current, map_current):

    print("\n=== Computing Future ===")

    results = []

    # cache ל-current כדי לא לחשב פעמיים
    current_cache = {}

    for i, row in enumerate(similarity_results):

        curr_tp = row["CurrentTP"]
        match_tp = row["MatchTP"]

        # =========================
        # HISTORY FUTURE (הכי חשוב)
        # =========================
        hist_future = get_future(df_history, map_history, match_tp)
        hist_ret = calc_return(hist_future)
        hist_up, hist_down = calc_max_move(hist_future)

        if hist_ret is None:
            continue

        # =========================
        # CURRENT FUTURE (השוואה בלבד)
        # =========================
        if curr_tp not in current_cache:

            curr_future = get_future(df_current, map_current, curr_tp)
            curr_ret = calc_return(curr_future)

            current_cache[curr_tp] = curr_ret

        curr_ret = current_cache[curr_tp]

        # =========================
        # FINAL ROW
        # =========================
        results.append({
            "CurrentTP": curr_tp,
            "MatchTP": match_tp,
            "Score": row["Score"],
            "Distance": row["Distance"],

            # 🔥 הכי חשוב
            "HistReturn": hist_ret,

            # 🔥 לבדיקת אמת
            "CurrReturn": curr_ret,

            # 🔥 מתקדם (יעזור בהמשך)
            "HistMaxUp": hist_up,
            "HistMaxDown": hist_down
        })

        if i % 50000 == 0:
            print(f"Processed {i}")

    print(f"Future computed: {len(results)}")

    return results