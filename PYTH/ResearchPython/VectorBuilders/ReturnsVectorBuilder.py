def extract(pattern):

    candles = pattern["candles"]

    # =========================
    # VALIDATION
    # =========================
    if len(candles) < 20:
        return None

    # =========================
    # CLOSE PRICES
    # =========================
    close_now = float(candles[-1]["close"])
    close_5 = float(candles[-5]["close"])
    close_20 = float(candles[-20]["close"])

    if close_5 == 0 or close_20 == 0:
        return None

    # =========================
    # RETURNS
    # =========================
    ret_5 = (close_now - close_5) / close_5
    ret_20 = (close_now - close_20) / close_20

    # =========================
    # OUTPUT SUB VECTOR
    # =========================
    return [
        float(ret_5),
        float(ret_20)
    ]