import numpy as np


# =========================
# CONFIG
# =========================
TLLEN = 50
GROUP_SIZE = 10

TIMEFRAMES = ["M1", "M15", "H1"]
LENS = [10, 30]

INDICATORS = ["RSI", "EMA"]


# =========================
# HELPERS
# =========================
def get_ordered_timepoints(candles):
    return sorted(candles.keys())


def slice_pattern(timepoints, start_idx):
    end_idx = start_idx + TLLEN
    if end_idx >= len(timepoints):
        return None
    return timepoints[start_idx:end_idx]


def build_segments(tp_slice):
    segments = []
    for i in range(0, len(tp_slice), GROUP_SIZE):
        seg = tp_slice[i:i + GROUP_SIZE]
        if len(seg) == GROUP_SIZE:
            segments.append(seg)
    return segments


# =========================
# PRICE FEATURES
# =========================
def price_features(segment, candles):

    prices = np.array([candles[tp]["close"] for tp in segment])

    return [
        np.mean(prices),
        np.min(prices),
        np.max(prices),
        prices[-1] - prices[0],
        np.std(prices)
    ]


# =========================
# RSI FEATURES
# =========================
def rsi_features(segment, indicators, tf, ln):

    values = []
    slopes = []

    for tp in segment:
        data = indicators[tp]["RSI"][tf][ln]
        values.append(data["value"])
        slopes.append(data["slope"])

    values = np.array(values)
    slopes = np.array(slopes)

    return [
        np.mean(values),
        values[-1] - values[0],
        np.mean(slopes)
    ]


# =========================
# EMA FEATURES
# =========================
def ema_features(segment, candles, indicators, tf, ln):

    values = []
    slopes = []
    distances = []

    for tp in segment:
        close = candles[tp]["close"]
        data = indicators[tp]["EMA"][tf][ln]

        ema_val = data["value"]

        values.append(ema_val)
        slopes.append(data["slope"])
        distances.append(ema_val - close)

    values = np.array(values)
    slopes = np.array(slopes)
    distances = np.array(distances)

    return [
        np.mean(distances),
        values[-1] - values[0],
        np.mean(slopes),
        np.std(values)
    ]


# =========================
# BUILD VECTOR FOR ONE TP
# =========================
def build_vector(tp, idx, timepoints, candles, indicators):

    tp_slice = slice_pattern(timepoints, idx)
    if tp_slice is None:
        return None

    segments = build_segments(tp_slice)

    features = []

    for seg in segments:

        # PRICE
        features.extend(price_features(seg, candles))

        # INDICATORS
        for tf in TIMEFRAMES:
            for ln in LENS:

                # RSI
                features.extend(
                    rsi_features(seg, indicators, tf, ln)
                )

                # EMA
                features.extend(
                    ema_features(seg, candles, indicators, tf, ln)
                )

    return np.array(features, dtype=np.float32)


# =========================
# MAIN BUILDER
# =========================
def build_all_vectors(candles, indicators):

    print("\n=== Building Vectors ===")

    timepoints = get_ordered_timepoints(candles)

    vectors = {}

    for i, tp in enumerate(timepoints):

        vec = build_vector(tp, i, timepoints, candles, indicators)

        if vec is not None:
            vectors[tp] = vec

        if i % 50000 == 0:
            print(f"Processed {i}")

    print(f"Vectors built: {len(vectors)}")

    return vectors