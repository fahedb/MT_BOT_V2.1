import numpy as np


# =========================
# CONFIG
# =========================
TIMEFRAME = "M1"
RSI_LEN = 30

MAX_SEGMENTS = 4
MIN_SEGMENT_LEN = 3

SMOOTH_EMA_LEN = 5


def extract(pattern):

    indicators = pattern["indicators"]
    pattern_timepoints = pattern.get("pattern_timepoints")
    window_size = pattern.get("window_size", len(pattern.get("candles", [])))

    # =========================
    # VALIDATION
    # =========================
    if not pattern_timepoints:
        return None

    if len(pattern_timepoints) < window_size:
        return None

    if window_size < MIN_SEGMENT_LEN:
        return None

    rsi_values = []

    try:
        window_tps = pattern_timepoints[-window_size:]

        for tp in window_tps:

            if tp not in indicators:
                return None

            rsi_node = indicators[tp]["RSI"][TIMEFRAME][RSI_LEN]
            rsi_values.append(float(rsi_node["value"]))

    except KeyError:
        return None

    if len(rsi_values) != window_size:
        return None

    # =========================
    # NORMALIZE RSI
    # =========================
    rsi = np.array(rsi_values, dtype=float)

    # Converts RSI from [0..100] into roughly [-1..1]
    rsi_norm = (rsi - 50.0) / 50.0

    # =========================
    # SMOOTH RSI
    # =========================
    rsi_smooth = _ema_smooth(rsi_norm, SMOOTH_EMA_LEN)

    # =========================
    # BUILD SEGMENTS
    # =========================
    segments = _build_segments(rsi_smooth)

    if len(segments) == 0:
        return None

    segments = _reduce_segments_to_max(segments, MAX_SEGMENTS)

    # =========================
    # OUTPUT SUB VECTOR
    # =========================
    sub_vector = []

    for seg_start, seg_end in segments:

        smooth_seg = rsi_smooth[seg_start:seg_end + 1]
        raw_seg = rsi_norm[seg_start:seg_end + 1]

        features = _segment_features(
            smooth_seg=smooth_seg,
            raw_seg=raw_seg,
            total_len=len(rsi_smooth)
        )

        sub_vector.extend(features)

    # Padding if less than MAX_SEGMENTS
    while len(sub_vector) < MAX_SEGMENTS * 7:
        sub_vector.extend([0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])

    return [float(x) for x in sub_vector]


def _ema_smooth(values, length):
    values = np.array(values, dtype=float)

    if len(values) == 0:
        return values

    alpha = 2.0 / (length + 1.0)

    result = np.zeros_like(values)
    result[0] = values[0]

    for i in range(1, len(values)):
        result[i] = alpha * values[i] + (1.0 - alpha) * result[i - 1]

    return result


def _build_segments(values):
    slopes = np.diff(values)

    if len(slopes) == 0:
        return []

    abs_slopes = np.abs(slopes)

    flat_threshold = max(np.std(slopes) * 0.25, 1e-6)
    aggressive_threshold = max(np.median(abs_slopes) * 1.50, flat_threshold * 2.0)

    states = []

    for slope in slopes:
        abs_slope = abs(slope)

        if abs_slope <= flat_threshold:
            state = 0
        else:
            direction = 1 if slope > 0 else -1
            strength = 2 if abs_slope >= aggressive_threshold else 1
            state = direction * strength

        states.append(state)

    segments = []
    start_idx = 0
    current_state = states[0]

    for i in range(1, len(states)):

        if states[i] != current_state:
            end_idx = i

            if (end_idx - start_idx + 1) >= MIN_SEGMENT_LEN:
                segments.append((start_idx, end_idx))

            start_idx = i
            current_state = states[i]

    last_start = start_idx
    last_end = len(values) - 1

    if (last_end - last_start + 1) >= MIN_SEGMENT_LEN:
        segments.append((last_start, last_end))

    if len(segments) == 0:
        segments.append((0, len(values) - 1))

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
            left_len = segments[weakest_idx - 1][1] - segments[weakest_idx - 1][0] + 1
            right_len = segments[weakest_idx + 1][1] - segments[weakest_idx + 1][0] + 1

            merge_with = weakest_idx - 1 if left_len <= right_len else weakest_idx + 1

        first = min(weakest_idx, merge_with)
        second = max(weakest_idx, merge_with)

        merged = (segments[first][0], segments[second][1])

        segments[first] = merged
        del segments[second]

    return segments


def _find_weakest_segment_index(segments):
    weakest_idx = 0
    weakest_score = None

    for i, (start, end) in enumerate(segments):

        length = end - start + 1

        # Short segments are considered weaker
        importance = length

        if weakest_score is None or importance < weakest_score:
            weakest_score = importance
            weakest_idx = i

    return weakest_idx


def _segment_features(smooth_seg, raw_seg, total_len):
    smooth_seg = np.array(smooth_seg, dtype=float)
    raw_seg = np.array(raw_seg, dtype=float)

    n = len(smooth_seg)

    if n == 0:
        return [0.0] * 7

    length_ratio = n / total_len
    avg_level = float(np.mean(smooth_seg))

    delta_norm = smooth_seg[-1] - smooth_seg[0]
    avg_slope = _avg_slope(smooth_seg)

    # Curvature + RMSE on smoothed RSI
    if n >= 3:
        x = np.linspace(0.0, 1.0, n)
        coefs = np.polyfit(x, smooth_seg, deg=2)

        curvature = coefs[0]

        pred = np.polyval(coefs, x)
        rmse = np.sqrt(np.mean((smooth_seg - pred) ** 2))

    elif n >= 2:
        curvature = 0.0

        x = np.linspace(0.0, 1.0, n)
        coefs = np.polyfit(x, smooth_seg, deg=1)

        pred = np.polyval(coefs, x)
        rmse = np.sqrt(np.mean((smooth_seg - pred) ** 2))

    else:
        curvature = 0.0
        rmse = 0.0

    # Difference between raw RSI and smoothed RSI inside the segment
    raw_noise = np.sqrt(np.mean((raw_seg - smooth_seg) ** 2))

    return [
        float(length_ratio),
        float(avg_slope),
        float(curvature),
        float(delta_norm),
        float(rmse),
        float(avg_level),
        float(raw_noise),
    ]


def _avg_slope(seg):
    if len(seg) < 2:
        return 0.0

    return (seg[-1] - seg[0]) / (len(seg) - 1)