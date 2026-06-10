import numpy as np


class VectorPipeline:
    def __init__(self, extractors, window_size=50):
        self.extractors = extractors
        self.window_size = window_size

    def build_all_vectors(self, candles, indicators):
        print("\n=== BUILD VECTORS PIPELINE ===")

        timepoints = sorted(candles.keys())
        index_map = {tp: i for i, tp in enumerate(timepoints)}

        vectors = {}

        for i, tp in enumerate(timepoints):
            pattern = self._build_pattern(
                tp=tp,
                candles=candles,
                indicators=indicators,
                index_map=index_map,
                timepoints=timepoints
            )

            if pattern is None:
                continue

            full_vector = []
            valid = True

            for extractor in self.extractors:
                sub_vector = extractor(pattern)

                if sub_vector is None:
                    valid = False
                    break

                full_vector.extend(sub_vector)

            if not valid:
                continue

            vectors[int(tp)] = np.array(full_vector, dtype=np.float32)

            if i % 50000 == 0:
                print(f"Built vectors: {i}/{len(timepoints)}")

        print(f"Total vectors built: {len(vectors)}")

        return vectors

    def _build_pattern(self, tp, candles, indicators, index_map, timepoints):
        idx = index_map.get(tp)

        if idx is None:
            return None

        if idx < self.window_size - 1:
            return None

        window_tps = timepoints[idx - self.window_size + 1: idx + 1]

        pattern_candles = []

        for wtp in window_tps:
            candle = candles.get(wtp)

            if candle is None:
                return None

            pattern_candles.append(candle)

        return {
            "tp": tp,
            "index": idx,

            # Candles inside the pattern window
            "candles": pattern_candles,

            # TimePoints inside the pattern window
            "pattern_timepoints": window_tps,

            # Full context
            "indicators": indicators,
            "all_candles": candles,
            "timepoints": timepoints,
            "index_map": index_map,

            # Pipeline configuration
            "window_size": self.window_size,
        }