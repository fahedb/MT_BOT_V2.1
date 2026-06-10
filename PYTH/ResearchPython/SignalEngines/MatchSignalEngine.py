import json
import os
from datetime import datetime

import numpy as np
import pandas as pd

from CandlesLoaders.HistoryLoader import load_history as load_history_data
from FutureExtractor.FutureExtractor import get_future
from CalcFutureReturn.CalcFutureReturn import compute_future_stats


# =========================
# DEFAULT CONFIG
# =========================

DEFAULT_SIGNAL_CONFIG = {
    # Minimum number of valid historical matches required
    "min_matches_count": 5,

    # =========================
    # OLD RETURN-BASED FILTERS
    # =========================
    "min_direction_agreement": None,
    "min_weighted_direction_agreement": None,

    # Minimum absolute predicted return.
    # This still uses PredReturn, so it can be used as Top10/Top5 threshold
    # from EdgeFinder even when direction is based on first-hit.
    "min_abs_pred_return": None,

    # Optional distance filters
    "max_avg_distance": None,
    "max_min_distance": None,

    # Trading directions
    "allow_buy": True,
    "allow_sell": True,

    # Return prediction weighting method
    "weighting_method": "1_over_distance",

    # Direction method:
    #   "pred_return"          -> old behavior: BUY/SELL from sign(PredReturn)
    #   "first_hit"            -> BUY/SELL from regular first-hit majority
    #   "weighted_first_hit"   -> BUY/SELL from weighted first-hit majority
    "signal_direction_method": "pred_return",

    # If True, return opposite direction
    "contrarian": False,

    # =========================
    # FIRST HIT CONFIG
    # =========================
    # Used when signal_direction_method is "first_hit" or "weighted_first_hit".
    "future_direction_method": "first_hit_pips",
    "first_hit_pips": 50,
    "first_hit_future_window": 10,
    "pip_size": 0.01,

    # If both UP and DOWN are touched in the same candle, OHLC cannot tell
    # which came first. Options: "noise", "up_first", "down_first"
    "same_candle_policy": "noise",

    # True  = ignore NOISE matches when deciding majority direction.
    # False = NOISE participates in ratios, but cannot be predicted direction.
    "ignore_noise_in_first_hit_prediction": True,

    # Optional first-hit filters
    "min_first_hit_agreement": None,
    "min_weighted_first_hit_agreement": None,
    "min_valid_first_hit_count": None,
    "max_first_hit_noise_ratio": None,

    # =========================
    # EXPORT CONFIG
    # =========================
    "auto_export_signals": True,
    "signals_output_dir": "Signals",
    "signals_file_prefix": "signals",
    "export_csv": True,
    "export_json": True,
    "include_timestamp": True,
}


# =========================
# HELPERS
# =========================

def load_matches_file(matches_file):
    if not os.path.exists(matches_file):
        raise FileNotFoundError(f"Matches file not found: {matches_file}")

    with open(matches_file, "r") as f:
        data = json.load(f)

    if "matches" in data:
        metadata = data.get("metadata", {})
        matches = data["matches"]
        return metadata, matches

    return {}, data


def build_index_map(candles):
    timepoints = sorted(candles.keys())
    return {tp: i for i, tp in enumerate(timepoints)}


def get_candle_value(row, possible_names):
    """
    Supports dict / pandas Series.
    """
    for name in possible_names:
        if name in row:
            return float(row[name])

    raise KeyError(f"Missing candle value. Tried keys: {possible_names}. Row={row}")


def get_close_from_df(df, tp):
    row = df.loc[tp]
    return get_candle_value(row, ["close", "Close", "ClosePrice"])


def get_high_from_any_row(row):
    if isinstance(row, dict):
        return get_candle_value(row, ["high", "High", "HighPrice"])

    if hasattr(row, "index"):
        return get_candle_value(row, ["high", "High", "HighPrice"])

    # Fallback only. In this project first-hit should normally use df.iloc,
    # not get_future, because get_future returns close-only arrays.
    return float(row[1])


def get_low_from_any_row(row):
    if isinstance(row, dict):
        return get_candle_value(row, ["low", "Low", "LowPrice"])

    if hasattr(row, "index"):
        return get_candle_value(row, ["low", "Low", "LowPrice"])

    # Fallback only.
    return float(row[2])


def iter_future_rows(future_data):
    if future_data is None:
        return []

    if hasattr(future_data, "iterrows"):
        return [row for _, row in future_data.iterrows()]

    if isinstance(future_data, np.ndarray):
        if future_data.ndim == 1:
            return [future_data]

        return [future_data[i] for i in range(future_data.shape[0])]

    if isinstance(future_data, (list, tuple)):
        return list(future_data)

    raise TypeError(f"Unsupported future_data type: {type(future_data)}")


def direction_value_to_trade_direction(direction_value):
    if direction_value == 1:
        return "BUY"

    if direction_value == -1:
        return "SELL"

    return "NONE"


def compute_first_hit_direction(
    future_df,
    entry_price,
    target_pips,
    pip_size,
    same_candle_policy="noise"
):
    """
    Checks what was hit first in the future window:
        UP    = price touched entry + target
        DOWN  = price touched entry - target
        NOISE = neither side touched, or ambiguous same candle if policy=noise

    Important:
        This function expects future_df to contain real OHLC rows.
        Do not pass get_future(...) output here if get_future returns close-only arrays.
    """

    if future_df is None or len(future_df) == 0:
        return {
            "direction": "NOISE",
            "direction_value": 0,
            "hit_index": None,
            "hit_price": None,
            "hit_pips": 0.0,
        }

    target_price_distance = float(target_pips) * float(pip_size)

    up_price = float(entry_price) + target_price_distance
    down_price = float(entry_price) - target_price_distance

    for i, row in enumerate(iter_future_rows(future_df)):
        high = get_high_from_any_row(row)
        low = get_low_from_any_row(row)

        up_hit = high >= up_price
        down_hit = low <= down_price

        if up_hit and down_hit:
            if same_candle_policy == "up_first":
                return {
                    "direction": "UP",
                    "direction_value": 1,
                    "hit_index": int(i),
                    "hit_price": float(up_price),
                    "hit_pips": float(target_pips),
                }

            if same_candle_policy == "down_first":
                return {
                    "direction": "DOWN",
                    "direction_value": -1,
                    "hit_index": int(i),
                    "hit_price": float(down_price),
                    "hit_pips": float(-target_pips),
                }

            return {
                "direction": "NOISE",
                "direction_value": 0,
                "hit_index": int(i),
                "hit_price": None,
                "hit_pips": 0.0,
            }

        if up_hit:
            return {
                "direction": "UP",
                "direction_value": 1,
                "hit_index": int(i),
                "hit_price": float(up_price),
                "hit_pips": float(target_pips),
            }

        if down_hit:
            return {
                "direction": "DOWN",
                "direction_value": -1,
                "hit_index": int(i),
                "hit_price": float(down_price),
                "hit_pips": float(-target_pips),
            }

    return {
        "direction": "NOISE",
        "direction_value": 0,
        "hit_index": None,
        "hit_price": None,
        "hit_pips": 0.0,
    }


def compute_first_hit_for_tp(
    tp,
    df,
    index_map,
    timepoints,
    signal_config
):
    """
    Computes first-hit from the real OHLC dataframe.

    This intentionally does NOT use get_future(), because in your project
    get_future() returns close-only numpy arrays.
    """

    if tp not in index_map:
        return {
            "direction": "NOISE",
            "direction_value": 0,
            "hit_index": None,
            "hit_price": None,
            "hit_pips": 0.0,
        }

    try:
        entry_price = get_close_from_df(df, tp)
    except Exception:
        return {
            "direction": "NOISE",
            "direction_value": 0,
            "hit_index": None,
            "hit_price": None,
            "hit_pips": 0.0,
        }

    current_index = index_map[tp]
    future_window_size = int(signal_config.get("first_hit_future_window", 10))

    start_index = current_index + 1
    end_index = min(current_index + 1 + future_window_size, len(timepoints))

    if start_index >= end_index:
        return {
            "direction": "NOISE",
            "direction_value": 0,
            "hit_index": None,
            "hit_price": None,
            "hit_pips": 0.0,
        }

    # Fast positional slice with real OHLC columns.
    future_df = df.iloc[start_index:end_index]

    return compute_first_hit_direction(
        future_df=future_df,
        entry_price=entry_price,
        target_pips=float(signal_config.get("first_hit_pips", 50)),
        pip_size=float(signal_config.get("pip_size", 0.01)),
        same_candle_policy=signal_config.get("same_candle_policy", "noise"),
    )


def compute_first_hit_consistency(
    direction_values_arr,
    weights_arr=None,
    ignore_noise=True
):
    """
    direction_values_arr:
        1  = UP
        -1 = DOWN
        0  = NOISE
    """

    if direction_values_arr is None or len(direction_values_arr) == 0:
        return None

    direction_values_arr = np.array(direction_values_arr, dtype=np.int32)

    up_mask = direction_values_arr == 1
    down_mask = direction_values_arr == -1
    noise_mask = direction_values_arr == 0

    total_count = len(direction_values_arr)
    up_count = int(np.sum(up_mask))
    down_count = int(np.sum(down_mask))
    noise_count = int(np.sum(noise_mask))
    valid_count = up_count + down_count

    noise_ratio = float(noise_count / total_count) if total_count else 0.0

    if ignore_noise:
        denom = valid_count

        if denom <= 0:
            up_ratio = 0.0
            down_ratio = 0.0
            agreement = 0.0
            pred_direction = "NOISE"
            pred_value = 0
        else:
            up_ratio = float(up_count / denom)
            down_ratio = float(down_count / denom)
            agreement = max(up_ratio, down_ratio)

            if up_ratio > down_ratio:
                pred_direction = "UP"
                pred_value = 1
            elif down_ratio > up_ratio:
                pred_direction = "DOWN"
                pred_value = -1
            else:
                pred_direction = "NOISE"
                pred_value = 0
    else:
        up_ratio = float(up_count / total_count)
        down_ratio = float(down_count / total_count)
        agreement = max(up_ratio, down_ratio)

        if up_ratio > down_ratio:
            pred_direction = "UP"
            pred_value = 1
        elif down_ratio > up_ratio:
            pred_direction = "DOWN"
            pred_value = -1
        else:
            pred_direction = "NOISE"
            pred_value = 0

    weighted_up_ratio = up_ratio
    weighted_down_ratio = down_ratio
    weighted_agreement = agreement

    if weights_arr is not None and len(weights_arr) == len(direction_values_arr):
        weights_arr = np.array(weights_arr, dtype=np.float64)

        if ignore_noise:
            valid_mask = ~noise_mask
            weights_sum = float(np.sum(weights_arr[valid_mask]))

            if weights_sum > 0:
                weighted_up_ratio = float(np.sum(weights_arr[up_mask]) / weights_sum)
                weighted_down_ratio = float(np.sum(weights_arr[down_mask]) / weights_sum)
                weighted_agreement = max(weighted_up_ratio, weighted_down_ratio)
            else:
                weighted_up_ratio = 0.0
                weighted_down_ratio = 0.0
                weighted_agreement = 0.0
        else:
            weights_sum = float(np.sum(weights_arr))

            if weights_sum > 0:
                weighted_up_ratio = float(np.sum(weights_arr[up_mask]) / weights_sum)
                weighted_down_ratio = float(np.sum(weights_arr[down_mask]) / weights_sum)
                weighted_agreement = max(weighted_up_ratio, weighted_down_ratio)

    if weighted_up_ratio > weighted_down_ratio:
        weighted_pred_direction = "UP"
        weighted_pred_value = 1
    elif weighted_down_ratio > weighted_up_ratio:
        weighted_pred_direction = "DOWN"
        weighted_pred_value = -1
    else:
        weighted_pred_direction = "NOISE"
        weighted_pred_value = 0

    return {
        "hist_first_hit_up_ratio": float(up_ratio),
        "hist_first_hit_down_ratio": float(down_ratio),
        "hist_first_hit_noise_ratio": float(noise_ratio),
        "hist_first_hit_agreement": float(agreement),

        "weighted_hist_first_hit_up_ratio": float(weighted_up_ratio),
        "weighted_hist_first_hit_down_ratio": float(weighted_down_ratio),
        "weighted_hist_first_hit_agreement": float(weighted_agreement),

        "hist_first_hit_up_count": up_count,
        "hist_first_hit_down_count": down_count,
        "hist_first_hit_noise_count": noise_count,
        "hist_first_hit_valid_count": valid_count,

        "pred_first_hit_direction": pred_direction,
        "pred_first_hit_direction_value": pred_value,

        "weighted_pred_first_hit_direction": weighted_pred_direction,
        "weighted_pred_first_hit_direction_value": weighted_pred_value,
    }


def compute_history_consistency(hist_returns_arr, weights_arr=None):
    if hist_returns_arr is None or len(hist_returns_arr) == 0:
        return None

    positives = hist_returns_arr > 0
    negatives = hist_returns_arr < 0

    positive_ratio = float(np.mean(positives))
    negative_ratio = float(np.mean(negatives))
    direction_agreement = max(positive_ratio, negative_ratio)

    if weights_arr is not None and len(weights_arr) == len(hist_returns_arr):
        weights_sum = float(np.sum(weights_arr))

        if weights_sum > 0:
            weighted_positive_ratio = float(np.sum(weights_arr[positives]) / weights_sum)
            weighted_negative_ratio = float(np.sum(weights_arr[negatives]) / weights_sum)
            weighted_direction_agreement = max(
                weighted_positive_ratio,
                weighted_negative_ratio
            )
        else:
            weighted_positive_ratio = positive_ratio
            weighted_negative_ratio = negative_ratio
            weighted_direction_agreement = direction_agreement
    else:
        weighted_positive_ratio = positive_ratio
        weighted_negative_ratio = negative_ratio
        weighted_direction_agreement = direction_agreement

    mean_return = float(np.mean(hist_returns_arr))
    median_return = float(np.median(hist_returns_arr))
    std_return = float(np.std(hist_returns_arr))

    if std_return <= 1e-12:
        consistency_score = 0.0
    else:
        consistency_score = abs(mean_return) / std_return

    return {
        "hist_positive_ratio": positive_ratio,
        "hist_negative_ratio": negative_ratio,
        "hist_direction_agreement": direction_agreement,

        "weighted_hist_positive_ratio": weighted_positive_ratio,
        "weighted_hist_negative_ratio": weighted_negative_ratio,
        "weighted_hist_direction_agreement": weighted_direction_agreement,

        "hist_return_mean": mean_return,
        "hist_return_median": median_return,
        "hist_return_std": std_return,
        "hist_consistency_score": consistency_score,
    }


def _make_json_safe(value):
    """
    Converts numpy/pandas objects into JSON-safe Python values.
    """
    if isinstance(value, (np.integer,)):
        return int(value)

    if isinstance(value, (np.floating,)):
        return float(value)

    if isinstance(value, (np.ndarray,)):
        return value.tolist()

    if isinstance(value, (list, dict, tuple)):
        return value

    try:
        if pd.isna(value):
            return None
    except Exception:
        pass

    return value


# =========================
# MATCH SIGNAL ENGINE
# =========================

class MatchSignalEngine:
    def __init__(
        self,
        matches_file,
        signal_config=None,
        history_candles=None
    ):
        self.matches_file = matches_file
        self.signal_config = dict(DEFAULT_SIGNAL_CONFIG)

        if signal_config is not None:
            self.signal_config.update(signal_config)

        self.metadata, self.matches = load_matches_file(matches_file)

        if history_candles is None:
            self.history_candles, _ = load_history_data()
        else:
            self.history_candles = history_candles

        self.history_index_map = build_index_map(self.history_candles)
        self.history_df = pd.DataFrame.from_dict(self.history_candles, orient="index")
        self.history_timepoints = list(self.history_df.index)

        # Signal cache: curr_tp -> signal dict
        self.signal_cache = {}

        # First-hit cache: hist_match_tp -> first-hit result
        self.first_hit_cache = {}

        print("\n=== MATCH SIGNAL ENGINE INIT ===")
        print(f"Matches file: {matches_file}")
        print(f"Total current patterns in matches: {len(self.matches)}")
        print(f"Signal config: {self.signal_config}")

    # =========================
    # PUBLIC API
    # =========================

    def get_signal(self, current_tp):
        current_tp = int(current_tp)

        if current_tp in self.signal_cache:
            return self.signal_cache[current_tp]

        signal = self._build_signal_for_tp(current_tp)
        self.signal_cache[current_tp] = signal

        return signal

    def has_signal_for_tp(self, current_tp):
        return str(int(current_tp)) in self.matches

    def get_available_timepoints(self):
        return sorted([int(tp) for tp in self.matches.keys()])

    def precompute_all_signals(self):
        print("\n=== PRECOMPUTE ALL SIGNALS ===")

        count = 0
        trade_count = 0

        for i, curr_tp_str in enumerate(self.matches.keys()):
            curr_tp = int(curr_tp_str)

            signal = self.get_signal(curr_tp)

            count += 1

            if signal["should_trade"]:
                trade_count += 1

            if i % 500 == 0:
                print(f"Precomputed {i}/{len(self.matches)}")

        print(f"Total signals checked: {count}")
        print(f"Tradable signals: {trade_count}")

        return self.signal_cache

    def build_signals_dataframe(self):
        rows = []

        for tp in self.get_available_timepoints():
            signal = self.get_signal(tp)
            row = dict(signal)

            if "valid_match_tps" in row:
                row["valid_match_tps_json"] = json.dumps(row["valid_match_tps"])
                del row["valid_match_tps"]

            row["matches_file"] = self.matches_file
            row["signal_config_json"] = json.dumps(self.signal_config, sort_keys=True)
            row["match_run_id"] = self.metadata.get("match_run_id")

            rows.append(row)

        return pd.DataFrame(rows)

    def export_signals(
        self,
        output_dir=None,
        file_prefix=None,
        export_csv=None,
        export_json=None,
        include_timestamp=None
    ):
        if output_dir is None:
            output_dir = self.signal_config.get("signals_output_dir", "Signals")

        if file_prefix is None:
            file_prefix = self.signal_config.get("signals_file_prefix", "signals")

        if export_csv is None:
            export_csv = self.signal_config.get("export_csv", True)

        if export_json is None:
            export_json = self.signal_config.get("export_json", True)

        if include_timestamp is None:
            include_timestamp = self.signal_config.get("include_timestamp", True)

        os.makedirs(output_dir, exist_ok=True)

        df = self.build_signals_dataframe()

        safe_prefix = str(file_prefix).replace(" ", "_").replace("/", "_").replace("\\", "_")

        if include_timestamp:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            base_filename = f"{safe_prefix}_{timestamp}"
        else:
            base_filename = safe_prefix

        base_path = os.path.join(output_dir, base_filename)

        csv_path = None
        json_path = None

        if export_csv:
            csv_path = f"{base_path}.csv"
            df.to_csv(csv_path, index=False)

        if export_json:
            json_path = f"{base_path}.json"
            records = df.to_dict(orient="records")

            safe_records = []
            for record in records:
                safe_record = {
                    key: _make_json_safe(value)
                    for key, value in record.items()
                }
                safe_records.append(safe_record)

            with open(json_path, "w") as f:
                json.dump(safe_records, f, indent=2)

        tradable_count = int(df["should_trade"].sum()) if "should_trade" in df.columns else 0
        total_count = int(len(df))

        print("\n=== SIGNALS EXPORTED ===")
        print(f"Total rows: {total_count}")
        print(f"Tradable signals: {tradable_count}")

        if csv_path:
            print(f"CSV: {csv_path}")

        if json_path:
            print(f"JSON: {json_path}")

        return {
            "rows": total_count,
            "tradable_signals": tradable_count,
            "dataframe": df,
            "csv_path": csv_path,
            "json_path": json_path,
        }

    def run_and_export_signals(self):
        self.precompute_all_signals()

        if self.signal_config.get("auto_export_signals", True):
            return self.export_signals()

        df = self.build_signals_dataframe()
        tradable_count = int(df["should_trade"].sum()) if "should_trade" in df.columns else 0

        return {
            "rows": int(len(df)),
            "tradable_signals": tradable_count,
            "dataframe": df,
            "csv_path": None,
            "json_path": None,
        }

    # =========================
    # INTERNAL SIGNAL BUILDING
    # =========================

    def _build_signal_for_tp(self, current_tp):
        curr_key = str(int(current_tp))

        if curr_key not in self.matches:
            return self._no_signal(
                current_tp=current_tp,
                reason="CurrentTP not found in matches file"
            )

        match_list = self.matches[curr_key]

        hist_returns = []
        hist_first_hit_values = []
        distances = []
        valid_match_tps = []

        use_first_hit = self._uses_first_hit_direction()

        for m in match_list:
            match_tp = int(m["MatchTP"])
            distance = float(m["Distance"])

            hist_future = get_future(
                match_tp,
                self.history_df,
                self.history_index_map
            )

            hist_stats = compute_future_stats(hist_future)

            if hist_stats is None:
                continue

            hist_return = float(hist_stats["Return"])

            hist_returns.append(hist_return)
            distances.append(distance)
            valid_match_tps.append(match_tp)

            if use_first_hit:
                first_hit = self._get_first_hit_for_match_tp(match_tp)
                hist_first_hit_values.append(int(first_hit["direction_value"]))

        if len(hist_returns) == 0:
            return self._no_signal(
                current_tp=current_tp,
                reason="No valid historical future returns"
            )

        hist_returns_arr = np.array(hist_returns, dtype=np.float64)
        distances_arr = np.array(distances, dtype=np.float64)

        pred_return, weights = self._compute_pred_return(
            hist_returns_arr=hist_returns_arr,
            distances_arr=distances_arr
        )

        consistency = compute_history_consistency(
            hist_returns_arr=hist_returns_arr,
            weights_arr=weights
        )

        if consistency is None:
            return self._no_signal(
                current_tp=current_tp,
                reason="Failed to compute return consistency"
            )

        first_hit_consistency = None
        if use_first_hit:
            first_hit_consistency = compute_first_hit_consistency(
                direction_values_arr=np.array(hist_first_hit_values, dtype=np.int32),
                weights_arr=weights,
                ignore_noise=self.signal_config.get("ignore_noise_in_first_hit_prediction", True)
            )

            if first_hit_consistency is None:
                return self._no_signal(
                    current_tp=current_tp,
                    reason="Failed to compute first-hit consistency"
                )

        avg_distance = float(np.mean(distances_arr))
        min_distance = float(np.min(distances_arr))
        matches_count = int(len(hist_returns_arr))

        base_signal = {
            "should_trade": False,
            "direction": "NONE",
            "current_tp": int(current_tp),

            # Return prediction remains available even when direction is first-hit.
            "pred_return": float(pred_return),
            "abs_pred_return": float(abs(pred_return)),

            "matches_count": matches_count,
            "avg_distance": avg_distance,
            "min_distance": min_distance,

            "hist_positive_ratio": consistency["hist_positive_ratio"],
            "hist_negative_ratio": consistency["hist_negative_ratio"],
            "hist_direction_agreement": consistency["hist_direction_agreement"],

            "weighted_hist_positive_ratio": consistency["weighted_hist_positive_ratio"],
            "weighted_hist_negative_ratio": consistency["weighted_hist_negative_ratio"],
            "weighted_hist_direction_agreement": consistency["weighted_hist_direction_agreement"],

            "hist_return_mean": consistency["hist_return_mean"],
            "hist_return_median": consistency["hist_return_median"],
            "hist_return_std": consistency["hist_return_std"],
            "hist_consistency_score": consistency["hist_consistency_score"],

            # First-hit fields are included in every signal for CSV consistency.
            "hist_first_hit_up_ratio": None,
            "hist_first_hit_down_ratio": None,
            "hist_first_hit_noise_ratio": None,
            "hist_first_hit_agreement": None,

            "weighted_hist_first_hit_up_ratio": None,
            "weighted_hist_first_hit_down_ratio": None,
            "weighted_hist_first_hit_agreement": None,

            "hist_first_hit_up_count": None,
            "hist_first_hit_down_count": None,
            "hist_first_hit_noise_count": None,
            "hist_first_hit_valid_count": None,

            "pred_first_hit_direction": None,
            "pred_first_hit_direction_value": None,

            "weighted_pred_first_hit_direction": None,
            "weighted_pred_first_hit_direction_value": None,

            "signal_direction_method": self.signal_config.get("signal_direction_method", "pred_return"),

            "valid_match_tps": valid_match_tps,
            "reason": "",
        }

        if first_hit_consistency is not None:
            base_signal.update(first_hit_consistency)

        passed, fail_reason = self._passes_filters(base_signal)

        if not passed:
            base_signal["reason"] = fail_reason
            return base_signal

        direction = self._resolve_signal_direction(base_signal)

        if direction == "NONE":
            base_signal["reason"] = "Signal direction is NONE"
            return base_signal

        if self.signal_config.get("contrarian") is True:
            direction = self._opposite_direction(direction)

        if direction == "BUY" and not self.signal_config.get("allow_buy", True):
            base_signal["reason"] = "BUY signal blocked by config"
            return base_signal

        if direction == "SELL" and not self.signal_config.get("allow_sell", True):
            base_signal["reason"] = "SELL signal blocked by config"
            return base_signal

        base_signal["should_trade"] = True
        base_signal["direction"] = direction
        base_signal["reason"] = "Signal passed all filters"

        return base_signal

    # =========================
    # PREDICTION / FIRST HIT
    # =========================

    def _compute_pred_return(self, hist_returns_arr, distances_arr):
        weighting_method = self.signal_config.get("weighting_method", "1_over_distance")

        if weighting_method == "1_over_distance":
            weights = 1.0 / (distances_arr + 1e-9)
            pred_return = float(np.average(hist_returns_arr, weights=weights))
            return pred_return, weights

        if weighting_method == "equal":
            weights = np.ones_like(hist_returns_arr, dtype=np.float64)
            pred_return = float(np.mean(hist_returns_arr))
            return pred_return, weights

        raise ValueError(f"Unsupported weighting_method: {weighting_method}")

    def _uses_first_hit_direction(self):
        method = self.signal_config.get("signal_direction_method", "pred_return")
        return method in ["first_hit", "weighted_first_hit"]

    def _get_first_hit_for_match_tp(self, match_tp):
        match_tp = int(match_tp)

        if match_tp in self.first_hit_cache:
            return self.first_hit_cache[match_tp]

        first_hit = compute_first_hit_for_tp(
            tp=match_tp,
            df=self.history_df,
            index_map=self.history_index_map,
            timepoints=self.history_timepoints,
            signal_config=self.signal_config
        )

        self.first_hit_cache[match_tp] = first_hit
        return first_hit

    def _resolve_signal_direction(self, signal):
        method = self.signal_config.get("signal_direction_method", "pred_return")

        if method == "pred_return":
            return self._direction_from_pred_return(signal["pred_return"])

        if method == "first_hit":
            return direction_value_to_trade_direction(
                signal.get("pred_first_hit_direction_value")
            )

        if method == "weighted_first_hit":
            return direction_value_to_trade_direction(
                signal.get("weighted_pred_first_hit_direction_value")
            )

        raise ValueError(f"Unsupported signal_direction_method: {method}")

    # =========================
    # FILTERS
    # =========================

    def _passes_filters(self, signal):
        cfg = self.signal_config

        min_matches_count = cfg.get("min_matches_count")
        if min_matches_count is not None:
            if signal["matches_count"] < int(min_matches_count):
                return False, f"MatchesCount < {min_matches_count}"

        min_direction_agreement = cfg.get("min_direction_agreement")
        if min_direction_agreement is not None:
            if signal["hist_direction_agreement"] < float(min_direction_agreement):
                return False, f"HistDirectionAgreement < {min_direction_agreement}"

        min_weighted_direction_agreement = cfg.get("min_weighted_direction_agreement")
        if min_weighted_direction_agreement is not None:
            if signal["weighted_hist_direction_agreement"] < float(min_weighted_direction_agreement):
                return False, f"WeightedHistDirectionAgreement < {min_weighted_direction_agreement}"

        min_abs_pred_return = cfg.get("min_abs_pred_return")
        if min_abs_pred_return is not None:
            if signal["abs_pred_return"] < float(min_abs_pred_return):
                return False, f"AbsPredReturn < {min_abs_pred_return}"

        min_first_hit_agreement = cfg.get("min_first_hit_agreement")
        if min_first_hit_agreement is not None:
            if signal.get("hist_first_hit_agreement") is None:
                return False, "HistFirstHitAgreement is None"

            if signal["hist_first_hit_agreement"] < float(min_first_hit_agreement):
                return False, f"HistFirstHitAgreement < {min_first_hit_agreement}"

        min_weighted_first_hit_agreement = cfg.get("min_weighted_first_hit_agreement")
        if min_weighted_first_hit_agreement is not None:
            if signal.get("weighted_hist_first_hit_agreement") is None:
                return False, "WeightedHistFirstHitAgreement is None"

            if signal["weighted_hist_first_hit_agreement"] < float(min_weighted_first_hit_agreement):
                return False, f"WeightedHistFirstHitAgreement < {min_weighted_first_hit_agreement}"

        min_valid_first_hit_count = cfg.get("min_valid_first_hit_count")
        if min_valid_first_hit_count is not None:
            if signal.get("hist_first_hit_valid_count") is None:
                return False, "HistFirstHitValidCount is None"

            if signal["hist_first_hit_valid_count"] < int(min_valid_first_hit_count):
                return False, f"HistFirstHitValidCount < {min_valid_first_hit_count}"

        max_first_hit_noise_ratio = cfg.get("max_first_hit_noise_ratio")
        if max_first_hit_noise_ratio is not None:
            if signal.get("hist_first_hit_noise_ratio") is None:
                return False, "HistFirstHitNoiseRatio is None"

            if signal["hist_first_hit_noise_ratio"] > float(max_first_hit_noise_ratio):
                return False, f"HistFirstHitNoiseRatio > {max_first_hit_noise_ratio}"

        max_avg_distance = cfg.get("max_avg_distance")
        if max_avg_distance is not None:
            if signal["avg_distance"] > float(max_avg_distance):
                return False, f"AvgDistance > {max_avg_distance}"

        max_min_distance = cfg.get("max_min_distance")
        if max_min_distance is not None:
            if signal["min_distance"] > float(max_min_distance):
                return False, f"MinDistance > {max_min_distance}"

        return True, None

    # =========================
    # DIRECTIONS
    # =========================

    def _direction_from_pred_return(self, pred_return):
        if pred_return > 0:
            return "BUY"

        if pred_return < 0:
            return "SELL"

        return "NONE"

    def _opposite_direction(self, direction):
        if direction == "BUY":
            return "SELL"

        if direction == "SELL":
            return "BUY"

        return "NONE"

    def _no_signal(self, current_tp, reason):
        return {
            "should_trade": False,
            "direction": "NONE",
            "current_tp": int(current_tp),
            "reason": reason,

            "pred_return": None,
            "abs_pred_return": None,
            "matches_count": 0,
            "avg_distance": None,
            "min_distance": None,

            "hist_positive_ratio": None,
            "hist_negative_ratio": None,
            "hist_direction_agreement": None,

            "weighted_hist_positive_ratio": None,
            "weighted_hist_negative_ratio": None,
            "weighted_hist_direction_agreement": None,

            "hist_return_mean": None,
            "hist_return_median": None,
            "hist_return_std": None,
            "hist_consistency_score": None,

            "hist_first_hit_up_ratio": None,
            "hist_first_hit_down_ratio": None,
            "hist_first_hit_noise_ratio": None,
            "hist_first_hit_agreement": None,

            "weighted_hist_first_hit_up_ratio": None,
            "weighted_hist_first_hit_down_ratio": None,
            "weighted_hist_first_hit_agreement": None,

            "hist_first_hit_up_count": None,
            "hist_first_hit_down_count": None,
            "hist_first_hit_noise_count": None,
            "hist_first_hit_valid_count": None,

            "pred_first_hit_direction": None,
            "pred_first_hit_direction_value": None,

            "weighted_pred_first_hit_direction": None,
            "weighted_pred_first_hit_direction_value": None,

            "signal_direction_method": self.signal_config.get("signal_direction_method", "pred_return"),

            "valid_match_tps": [],
        }
