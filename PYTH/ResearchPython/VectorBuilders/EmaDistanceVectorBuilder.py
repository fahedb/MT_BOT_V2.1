def _get_ema(indicators, tp, timeframe, length):
    try:
        return indicators[tp]["EMA"][timeframe][length]
    except KeyError:
        return None


def _safe_div(numerator, denominator):
    if denominator == 0:
        return 0.0

    return numerator / denominator


def _calc_direction_score(values):
    if len(values) < 2:
        return None

    up_count = 0
    down_count = 0

    for i in range(1, len(values)):
        diff = values[i] - values[i - 1]

        if diff > 0:
            up_count += 1
        elif diff < 0:
            down_count += 1

    total = up_count + down_count

    if total == 0:
        return 0.0

    return (up_count - down_count) / total


def _build_timeframe_features(pattern, timeframe, length=20):
    indicators = pattern["indicators"]
    pattern_timepoints = pattern.get("pattern_timepoints")
    candles = pattern["candles"]

    if not pattern_timepoints or len(pattern_timepoints) < 10:
        return None

    if len(candles) != len(pattern_timepoints):
        return None

    window_size = pattern.get("window_size", len(pattern_timepoints))
    recent_size = max(5, int(window_size * 0.25))

    distances = []

    for i, tp in enumerate(pattern_timepoints):
        candle = candles[i]

        close_price = float(candle["close"])

        ema = _get_ema(
            indicators=indicators,
            tp=tp,
            timeframe=timeframe,
            length=length
        )

        if ema is None:
            return None

        ema_value = float(ema["value"])

        if ema_value == 0:
            return None

        # Positive = price above EMA
        # Negative = price below EMA
        close_dist_ema = _safe_div(close_price - ema_value, ema_value)

        distances.append(close_dist_ema)

    dist_now = distances[-1]
    dist_avg_full = sum(distances) / len(distances)
    dist_avg_recent = sum(distances[-recent_size:]) / len(distances[-recent_size:])

    dist_global_direction = _calc_direction_score(distances)
    dist_recent_direction = _calc_direction_score(distances[-recent_size:])

    if dist_global_direction is None or dist_recent_direction is None:
        return None

    return [
        float(dist_now),
        float(dist_avg_full),
        float(dist_avg_recent),
        float(dist_global_direction),
        float(dist_recent_direction),
    ]


def extract(pattern):

    m1_features = _build_timeframe_features(pattern, "M1", 20)
    m15_features = _build_timeframe_features(pattern, "M15", 20)

    if m1_features is None or m15_features is None:
        return None

    return m1_features + m15_features