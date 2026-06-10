def _get_rsi_value(indicators, tp, timeframe, length):
    try:
        return float(indicators[tp]["RSI"][timeframe][length]["value"])
    except KeyError:
        return None


def _normalize_rsi(rsi_value):
    # RSI 50 -> 0
    # RSI 70 -> +0.4
    # RSI 30 -> -0.4
    return (rsi_value - 50.0) / 50.0


def _build_timeframe_features(pattern, timeframe, length=20):
    indicators = pattern["indicators"]
    pattern_timepoints = pattern.get("pattern_timepoints")

    if not pattern_timepoints or len(pattern_timepoints) < 10:
        return None

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

    rsi_now = rsi_values[-1]
    rsi_avg = sum(rsi_values) / len(rsi_values)

    return [
        float(_normalize_rsi(rsi_now)),
        float(_normalize_rsi(rsi_avg)),
    ]


def extract(pattern):

    m1_features = _build_timeframe_features(pattern, "M1", 20)
    m15_features = _build_timeframe_features(pattern, "M15", 20)

    if m1_features is None or m15_features is None:
        return None

    return m1_features + m15_features