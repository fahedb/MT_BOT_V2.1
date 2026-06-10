def _get_rsi_value(indicators, tp, timeframe, length):
    try:
        return float(indicators[tp]["RSI"][timeframe][length]["value"])
    except KeyError:
        return None


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

    if not pattern_timepoints or len(pattern_timepoints) < 10:
        return None

    window_size = pattern.get("window_size", len(pattern_timepoints))

    recent_size = max(5, int(window_size * 0.25))

    rsi_values = []

    for tp in pattern_timepoints:
        value = _get_rsi_value(
            indicators=indicators,
            tp=tp,
            timeframe=timeframe,
            length=length
        )

        if value is None:
            return None

        rsi_values.append(value)

    global_direction = _calc_direction_score(rsi_values)
    recent_direction = _calc_direction_score(rsi_values[-recent_size:])

    if global_direction is None or recent_direction is None:
        return None

    return [
        float(global_direction),
        float(recent_direction),
    ]


def extract(pattern):

    m1_features = _build_timeframe_features(pattern, "M1", 20)
    m15_features = _build_timeframe_features(pattern, "M15", 20)

    if m1_features is None or m15_features is None:
        return None

    return m1_features + m15_features