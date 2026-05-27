import numpy as np


# =========================
# CONFIG
# =========================
TLLEN = 50      # אורך התבנית
VISION = 12     # כמה נרות קדימה


# =========================
# CORE FUNCTION
# =========================
def get_future(tp, df, time_to_index):
    """
    Extract future window from given TimePoint

    Parameters:
        tp (int): TimePoint
        df (DataFrame): candles dataframe
        time_to_index (dict): mapping TimePoint -> index

    Returns:
        np.array or None
    """

    # אין אינדקס → אין נתונים
    if tp not in time_to_index:
        return None

    idx = time_to_index[tp]

    # התחלה אחרי סוף התבנית
    start_idx = idx + TLLEN
    end_idx = start_idx + VISION

    # יציאה מגבולות
    if end_idx >= len(df):
        return None

    # שליפה
    future = df.iloc[start_idx:end_idx]["close"].values

    return future


# =========================
# BATCH EXTRACTION
# =========================
def extract_futures(tp_list, df, time_to_index):
    """
    Extract futures for multiple TimePoints

    Parameters:
        tp_list (list[int])
    
    Returns:
        dict {tp: future_vector}
    """

    futures = {}

    for i, tp in enumerate(tp_list):

        f = get_future(tp, df, time_to_index)

        if f is not None:
            futures[tp] = f

        if i % 10000 == 0:
            print(f"Processed {i}")

    print(f"Valid futures: {len(futures)}")

    return futures


# =========================
# DEBUG
# =========================
if __name__ == "__main__":

    print("FutureExtractor ready ✔")