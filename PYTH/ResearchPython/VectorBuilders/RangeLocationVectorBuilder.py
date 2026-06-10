def _safe_div(numerator, denominator):
    if denominator == 0:
        return 0.0

    return numerator / denominator


def _calc_range_stats(candles, window_size):
    if len(candles) < window_size:
        return None

    window = candles[-window_size:]

    close_price = float(window[-1]["close"])

    highs = [float(c["high"]) for c in window]
    lows = [float(c["low"]) for c in window]

    highest_high = max(highs)
    lowest_low = min(lows)

    price_range = highest_high - lowest_low

    if price_range <= 0 or close_price == 0:
        return None

    close_position = _safe_div(close_price - lowest_low, price_range)

    range_relative = _safe_div(price_range, close_price)

    return {
        "close_position": float(close_position),
        "range_relative": float(range_relative),
    }


def extract(pattern):

    candles = pattern["candles"]
    window_size = pattern.get("window_size", len(candles))

    if len(candles) < 10:
        return None

    short_window = max(5, int(window_size * 0.4))
    long_window = window_size

    short_stats = _calc_range_stats(candles, short_window)
    long_stats = _calc_range_stats(candles, long_window)

    if short_stats is None or long_stats is None:
        return None

    range_compression = _safe_div(
        short_stats["range_relative"],
        long_stats["range_relative"]
    )

    return [
        float(long_stats["close_position"]),
        float(long_stats["range_relative"]),
        float(range_compression),
    ]