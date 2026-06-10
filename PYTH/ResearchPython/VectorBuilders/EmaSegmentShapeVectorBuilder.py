import numpy as np


MAX_SEGMENTS = 4
MIN_SEGMENT_LEN = 3


def create_extractor(
    timeframe="H1",
    ema_len=30,
    max_segments=MAX_SEGMENTS,
    min_segment_len=MIN_SEGMENT_LEN
):
    def extract(pattern):
        return _extract(
            pattern=pattern,
            timeframe=timeframe,
            ema_len=ema_len,
            max_segments=max_segments,
            min_segment_len=min_segment_len
        )

    return extract


def _extract(
    pattern,
    timeframe,
    ema_len,
    max_segments,
    min_segment_len
):
    indicators = pattern["indicators"]
    pattern_timepoints = pattern.get("pattern_timepoints")
    window_size = pattern.get("window_size", 50)

    if not pattern_timepoints:
        return None

    if len(pattern_timepoints) < window_size:
        return None

    ema_values = []

    try:
        window_tps = pattern_timepoints[-window_size:]

        for candle_tp in window_tps:
            if candle_tp not in indicators:
                return None

            ema_node = indicators[candle_tp]["EMA"][timeframe][ema_len]
            ema_values.append(float(ema_node["value"]))

    except KeyError:
        return None

    if len(ema_values) != window_size:
        return None

    if ema_values[0] == 0:
        return None

    ema = np.array(ema_values, dtype=float)
    ema_norm = (ema - ema[0]) / ema[0]

    segments = _build_segments(
        values=ema_norm,
        min_segment_len=min_segment_len
    )

    if len(segments) == 0:
        return None

    segments = _reduce_segments_to_max(
        segments=segments,
        max_segments=max_segments
    )

    sub_vector = []

    for seg in segments:
        features = _segment_features(seg, total_len=len(ema_norm))
        sub_vector.extend(features)

    while len(sub_vector) < max_segments * 5:
        sub_vector.extend([0.0, 0.0, 0.0, 0.0, 0.0])

    return [float(x) for x in sub_vector]


def _build_segments(values, min_segment_len):
    slopes = np.diff(values)

    if len(slopes) == 0:
        return []

    abs_slopes = np.abs(slopes)

    flat_threshold = max(np.median(abs_slopes) * 0.25, 1e-8)

    states = []

    for s in slopes:
        abs_s = abs(s)

        if abs_s <= flat_threshold:
            state = 0
        else:
            state = 1 if s > 0 else -1

        states.append(state)

    segments = []
    start_idx = 0
    current_state = states[0]

    for i in range(1, len(states)):
        if states[i] != current_state:
            end_idx = i
            seg = values[start_idx:end_idx + 1]

            if len(seg) >= min_segment_len:
                segments.append(seg)

            start_idx = i
            current_state = states[i]

    last_seg = values[start_idx:len(values)]

    if len(last_seg) >= min_segment_len:
        segments.append(last_seg)

    if len(segments) == 0:
        segments.append(values)

    return segments


def _reduce_segments_to_max(segments, max_segments):
    segments = list(segments)

    while len(segments) > max_segments:
        weakest_idx = _find_weakest_segment_index(segments)

        if weakest_idx == 0:
            merge_with = 1
        elif weakest_idx == len(segments) - 1:
            merge_with = weakest_idx - 1
        else:
            left_slope = _avg_slope(segments[weakest_idx - 1])
            right_slope = _avg_slope(segments[weakest_idx + 1])
            current_slope = _avg_slope(segments[weakest_idx])

            if abs(current_slope - left_slope) <= abs(current_slope - right_slope):
                merge_with = weakest_idx - 1
            else:
                merge_with = weakest_idx + 1

        first = min(weakest_idx, merge_with)
        second = max(weakest_idx, merge_with)

        merged = np.concatenate([segments[first], segments[second][1:]])

        segments[first] = merged
        del segments[second]

    return segments


def _find_weakest_segment_index(segments):
    weakest_idx = 0
    weakest_score = None

    for i, seg in enumerate(segments):
        delta = abs(seg[-1] - seg[0])
        length = len(seg)

        importance = delta * length

        if weakest_score is None or importance < weakest_score:
            weakest_score = importance
            weakest_idx = i

    return weakest_idx


def _segment_features(seg, total_len):
    seg = np.array(seg, dtype=float)
    n = len(seg)

    length_ratio = n / total_len
    delta_norm = seg[-1] - seg[0]
    avg_slope = _avg_slope(seg)

    if n >= 3:
        x = np.linspace(0.0, 1.0, n)
        coefs = np.polyfit(x, seg, deg=2)

        curvature = coefs[0]

        pred = np.polyval(coefs, x)
        rmse = np.sqrt(np.mean((seg - pred) ** 2))

    elif n >= 2:
        curvature = 0.0
        x = np.linspace(0.0, 1.0, n)
        coefs = np.polyfit(x, seg, deg=1)

        pred = np.polyval(coefs, x)
        rmse = np.sqrt(np.mean((seg - pred) ** 2))

    else:
        curvature = 0.0
        rmse = 0.0

    return [
        float(length_ratio),
        float(avg_slope),
        float(curvature),
        float(delta_norm),
        float(rmse),
    ]


def _avg_slope(seg):
    if len(seg) < 2:
        return 0.0

    return (seg[-1] - seg[0]) / (len(seg) - 1)

def extract(pattern):
    return _extract(
        pattern=pattern,
        timeframe="M1",
        ema_len=30,
        max_segments=4,
        min_segment_len=3
    )