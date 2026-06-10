def _safe_div(numerator, denominator):
    if denominator == 0:
        return 0.0

    return numerator / denominator


def _calc_candle_parts(candle):
    open_price = float(candle["open"])
    close_price = float(candle["close"])
    high_price = float(candle["high"])
    low_price = float(candle["low"])

    candle_range = high_price - low_price

    if candle_range <= 0:
        return None

    body = abs(close_price - open_price)

    upper_wick = high_price - max(open_price, close_price)
    lower_wick = min(open_price, close_price) - low_price

    body_ratio = _safe_div(body, candle_range)
    upper_wick_ratio = _safe_div(upper_wick, candle_range)
    lower_wick_ratio = _safe_div(lower_wick, candle_range)
    close_position = _safe_div(close_price - low_price, candle_range)

    return {
        "range": candle_range,
        "body": body,
        "body_ratio": body_ratio,
        "upper_wick_ratio": upper_wick_ratio,
        "lower_wick_ratio": lower_wick_ratio,
        "close_position": close_position
    }


def extract(pattern):

    candles = pattern["candles"]

    # =========================
    # VALIDATION
    # =========================
    if len(candles) < 5:
        return None

    last_parts = _calc_candle_parts(candles[-1])

    if last_parts is None:
        return None

    recent_parts = []

    for candle in candles[-5:]:
        parts = _calc_candle_parts(candle)

        if parts is None:
            return None

        recent_parts.append(parts)

    # =========================
    # LAST CANDLE STRUCTURE
    # =========================
    body_ratio = last_parts["body_ratio"]
    upper_wick_ratio = last_parts["upper_wick_ratio"]
    lower_wick_ratio = last_parts["lower_wick_ratio"]
    close_position = last_parts["close_position"]

    # =========================
    # RECENT AVERAGES
    # =========================
    avg_body_ratio_5 = sum(p["body_ratio"] for p in recent_parts) / len(recent_parts)
    avg_range_5 = sum(p["range"] for p in recent_parts) / len(recent_parts)

    # =========================
    # OUTPUT SUB VECTOR
    # =========================
    return [
        float(body_ratio),
        float(upper_wick_ratio),
        float(lower_wick_ratio),
        float(close_position),
        float(avg_body_ratio_5),
        float(avg_range_5)
    ]