def _get_rsi_value(indicators, tp, timeframe, length):
    try:
        return float(indicators[tp]["RSI"][timeframe][length]["value"])
    except KeyError:
        return None


def _safe_div(numerator, denominator):
    if denominator == 0:
        return 0.0

    return numerator / denominator


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

    recent_values = rsi_values[-recent_size:]

    # =========================
    # EXTREME RATIOS
    # =========================
    full_overbought_ratio = _safe_div(
        sum(1 for v in rsi_values if v >= 70.0),
        len(rsi_values)
    )

    full_oversold_ratio = _safe_div(
        sum(1 for v in rsi_values if v <= 30.0),
        len(rsi_values)
    )

    recent_overbought_ratio = _safe_div(
        sum(1 for v in recent_values if v >= 70.0),
        len(recent_values)
    )

    recent_oversold_ratio = _safe_div(
        sum(1 for v in recent_values if v <= 30.0),
        len(recent_values)
    )

    # =========================
    # EXIT FROM EXTREME
    # =========================
    # Positive means RSI was overbought during the full pattern,
    # but less overbought recently.
    overbought_exit_pressure = full_overbought_ratio - recent_overbought_ratio

    # Positive means RSI was oversold during the full pattern,
    # but less oversold recently.
    oversold_exit_pressure = full_oversold_ratio - recent_oversold_ratio

    return [
        float(full_overbought_ratio),
        float(full_oversold_ratio),
        float(overbought_exit_pressure),
        float(oversold_exit_pressure),
    ]


def extract(pattern):

    m1_features = _build_timeframe_features(pattern, "M1", 20)
    m15_features = _build_timeframe_features(pattern, "M15", 20)

    if m1_features is None or m15_features is None:
        return None

    return m1_features + m15_features