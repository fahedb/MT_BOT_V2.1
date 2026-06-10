def extract(pattern):

    tp = pattern["tp"]
    candles = pattern["candles"]
    indicators = pattern["indicators"]

    # =========================
    # VALIDATION
    # =========================
    if tp not in indicators:
        return None

    if len(candles) == 0:
        return None

    close_price = float(candles[-1]["close"])

    try:
        ema_h1_10 = indicators[tp]["EMA"]["H1"][10]
        ema_h1_20 = indicators[tp]["EMA"]["H1"][20]
        ema_h1_30 = indicators[tp]["EMA"]["H1"][30]

    except KeyError:
        return None

    # =========================
    # VALUES
    # =========================
    ema10_value = float(ema_h1_10["value"])
    ema20_value = float(ema_h1_20["value"])
    ema30_value = float(ema_h1_30["value"])

    ema10_slope = float(ema_h1_10["slope"])
    ema20_slope = float(ema_h1_20["slope"])
    ema30_slope = float(ema_h1_30["slope"])

    if ema20_value == 0 or ema30_value == 0:
        return None

    # =========================
    # TREND FEATURES
    # =========================

    # Price location relative to trend
    close_dist_ema20 = (close_price - ema20_value) / ema20_value

    # Fast EMA relative to slow EMA
    ema10_vs_ema30 = (ema10_value - ema30_value) / ema30_value

    # EMA stacking:
    # positive means EMA10 > EMA20 > EMA30 tendency
    ema_stack_strength = ((ema10_value - ema20_value) + (ema20_value - ema30_value)) / ema30_value

    # Slope agreement:
    # positive = slopes are generally pointing up
    avg_slope = (ema10_slope + ema20_slope + ema30_slope) / 3.0

    # =========================
    # OUTPUT SUB VECTOR
    # =========================
    return [
        float(close_dist_ema20),
        float(ema10_vs_ema30),
        float(ema_stack_strength),
        float(avg_slope),
    ]