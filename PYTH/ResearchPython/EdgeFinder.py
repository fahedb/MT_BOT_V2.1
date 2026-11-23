import json
import os
import traceback

import pandas as pd
import numpy as np

from sqlalchemy import create_engine, text

from CandlesLoaders.HistoryLoader import load_history as load_history_data
from CandlesLoaders.CurrentLoader import load_history as load_current_data

from FutureExtractor.FutureExtractor import get_future
from CalcFutureReturn.CalcFutureReturn import compute_future_stats


# =========================
# CONFIG
# =========================
# Connection string: override with the MTBOT_SQL_CONN environment variable.
# See docs/CONFIGURATION.md (refactor plan Phase 0, finding E12).
CONN_STR = os.environ.get(
    "MTBOT_SQL_CONN",
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes",
)

MATCHES_DIR = "Matches"

# Manual default
MATCH_RUN_ID = 39

DEFAULT_EVALUATION_CONFIG = {
    "prediction_method": "weighted_average",
    "weighting_method": "1_over_distance",
    "future_stats_function": "compute_future_stats_default",

    # =========================
    # RETURN CONSISTENCY FILTERS
    # =========================
    "min_direction_agreement": None,
    "min_weighted_direction_agreement": None,
    "min_consistency_score": None,
    "max_return_std": None,
    "min_matches_count": None,

    # =========================
    # FIRST HIT CONFIG
    # =========================
    # "return_sign"      = old behavior: direction by final Return sign
    # "first_hit_pips"   = new behavior: direction by first +X/-X pips hit
    "future_direction_method": "first_hit_pips",

    # Used only when future_direction_method == "first_hit_pips"
    "first_hit_pips": 150,
    "pip_size": 0.01,
    "first_hit_future_window" : 15,

    # If both UP and DOWN are touched in the same candle, OHLC cannot tell
    # which came first. Options: "noise", "up_first", "down_first"
    "same_candle_policy": "noise",

    # When predicting from historical matches:
    # True  = ignore NOISE matches when deciding majority direction
    # False = NOISE participates in ratios, but cannot be predicted direction
    "ignore_noise_in_first_hit_prediction": True,

    # If False, rows where current future is NOISE are excluded from accuracy calculations
    "include_noise_in_accuracy": False,

    # Optional first-hit filters
    "min_first_hit_agreement": 0.7,
    "min_weighted_first_hit_agreement": None,
    "min_valid_first_hit_count": 7,
    "max_first_hit_noise_ratio": 0.3,

    # If True, first-hit prediction uses weighted majority instead of regular majority
    "use_weighted_first_hit_prediction": False
}


# =========================
# DB HELPERS
# =========================
def create_engine_instance():
    return create_engine(CONN_STR)


def get_match_run_file_from_db(match_run_id):
    engine = create_engine_instance()

    query = text("""
        SELECT MatchesFile
        FROM dbo.VectorMatchRuns
        WHERE MatchRunID = :MatchRunID
    """)

    with engine.begin() as conn:
        row = conn.execute(query, {
            "MatchRunID": match_run_id
        }).fetchone()

    if row is None:
        raise ValueError(f"MatchRunID not found in DB: {match_run_id}")

    matches_file = row[0]

    if matches_file is None or matches_file == "":
        raise ValueError(f"MatchRunID={match_run_id} has no MatchesFile")

    return matches_file


def insert_edge_run_to_db(
    match_run_id,
    evaluation_config,
    debug_metrics,
    edge_df,
    baseline_return,
    dataset_rows
):
    engine = create_engine_instance()

    evaluation_config_json = json.dumps(evaluation_config, sort_keys=True)

    query = text("""
        INSERT INTO dbo.VectorEdgeRuns
        (
            MatchRunID,
            EvaluationConfigJson,

            DatasetRows,

            PredReturnCorr,
            AvgHistReturnCorr,
            MedianHistReturnCorr,

            DirectionAccuracy,
            BaselineReturn,

            Top50Accuracy,
            Top50Samples,
            Top50AvgReturn,
            Top50Threshold,

            Top40Accuracy,
            Top40Samples,
            Top40AvgReturn,
            Top40Threshold,

            Top30Accuracy,
            Top30Samples,
            Top30AvgReturn,
            Top30Threshold,

            Top20Accuracy,
            Top20Samples,
            Top20AvgReturn,
            Top20Threshold,

            Top10Accuracy,
            Top10Samples,
            Top10AvgReturn,
            Top10Threshold,

            Top5Accuracy,
            Top5Samples,
            Top5AvgReturn,
            Top5Threshold
        )
        OUTPUT INSERTED.EdgeRunID
        VALUES
        (
            :MatchRunID,
            :EvaluationConfigJson,

            :DatasetRows,

            :PredReturnCorr,
            :AvgHistReturnCorr,
            :MedianHistReturnCorr,

            :DirectionAccuracy,
            :BaselineReturn,

            :Top50Accuracy,
            :Top50Samples,
            :Top50AvgReturn,
            :Top50Threshold,

            :Top40Accuracy,
            :Top40Samples,
            :Top40AvgReturn,
            :Top40Threshold,

            :Top30Accuracy,
            :Top30Samples,
            :Top30AvgReturn,
            :Top30Threshold,

            :Top20Accuracy,
            :Top20Samples,
            :Top20AvgReturn,
            :Top20Threshold,

            :Top10Accuracy,
            :Top10Samples,
            :Top10AvgReturn,
            :Top10Threshold,

            :Top5Accuracy,
            :Top5Samples,
            :Top5AvgReturn,
            :Top5Threshold
        )
    """)

    params = {
        "MatchRunID": match_run_id,
        "EvaluationConfigJson": evaluation_config_json,

        "DatasetRows": int(dataset_rows),

        "PredReturnCorr": debug_metrics["PredReturnCorr"],
        "AvgHistReturnCorr": debug_metrics["AvgHistReturnCorr"],
        "MedianHistReturnCorr": debug_metrics["MedianHistReturnCorr"],

        "DirectionAccuracy": debug_metrics["DirectionAccuracy"],
        "BaselineReturn": baseline_return,

        "Top50Accuracy": get_edge_metric(edge_df, "Top50", "DirectionAccuracy"),
        "Top50Samples": get_edge_metric(edge_df, "Top50", "Samples"),
        "Top50AvgReturn": get_edge_metric(edge_df, "Top50", "AvgCurrReturn"),
        "Top50Threshold": get_edge_metric(edge_df, "Top50", "Threshold"),

        "Top40Accuracy": get_edge_metric(edge_df, "Top40", "DirectionAccuracy"),
        "Top40Samples": get_edge_metric(edge_df, "Top40", "Samples"),
        "Top40AvgReturn": get_edge_metric(edge_df, "Top40", "AvgCurrReturn"),
        "Top40Threshold": get_edge_metric(edge_df, "Top40", "Threshold"),

        "Top30Accuracy": get_edge_metric(edge_df, "Top30", "DirectionAccuracy"),
        "Top30Samples": get_edge_metric(edge_df, "Top30", "Samples"),
        "Top30AvgReturn": get_edge_metric(edge_df, "Top30", "AvgCurrReturn"),
        "Top30Threshold": get_edge_metric(edge_df, "Top30", "Threshold"),

        "Top20Accuracy": get_edge_metric(edge_df, "Top20", "DirectionAccuracy"),
        "Top20Samples": get_edge_metric(edge_df, "Top20", "Samples"),
        "Top20AvgReturn": get_edge_metric(edge_df, "Top20", "AvgCurrReturn"),
        "Top20Threshold": get_edge_metric(edge_df, "Top20", "Threshold"),

        "Top10Accuracy": get_edge_metric(edge_df, "Top10", "DirectionAccuracy"),
        "Top10Samples": get_edge_metric(edge_df, "Top10", "Samples"),
        "Top10AvgReturn": get_edge_metric(edge_df, "Top10", "AvgCurrReturn"),
        "Top10Threshold": get_edge_metric(edge_df, "Top10", "Threshold"),

        "Top5Accuracy": get_edge_metric(edge_df, "Top5", "DirectionAccuracy"),
        "Top5Samples": get_edge_metric(edge_df, "Top5", "Samples"),
        "Top5AvgReturn": get_edge_metric(edge_df, "Top5", "AvgCurrReturn"),
        "Top5Threshold": get_edge_metric(edge_df, "Top5", "Threshold"),
    }

    with engine.begin() as conn:
        edge_run_id = conn.execute(query, params).scalar()

    print(f"✅ Saved EdgeRunID={edge_run_id} to DB")

    return int(edge_run_id)


# =========================
# LOAD MATCHES
# =========================
def load_matches(path):
    with open(path, "r") as f:
        data = json.load(f)

    if "matches" in data:
        metadata = data.get("metadata", {})
        matches = data["matches"]
        return metadata, matches

    return {}, data


# =========================
# BUILD INDEX MAP
# =========================
def build_index_map(candles):
    timepoints = sorted(candles.keys())
    return {tp: i for i, tp in enumerate(timepoints)}


# =========================
# CANDLE HELPERS
# =========================
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
    """
    Supports:
        - pandas Series / dict with high keys
        - numpy array/list with assumed OHLC order: [open, high, low, close]

    If your get_future ndarray order is different, change row[1].
    """
    if isinstance(row, dict):
        return get_candle_value(row, ["high", "High", "HighPrice"])

    if hasattr(row, "index"):
        return get_candle_value(row, ["high", "High", "HighPrice"])

    return float(row[1])


def get_low_from_any_row(row):
    """
    Supports:
        - pandas Series / dict with low keys
        - numpy array/list with assumed OHLC order: [open, high, low, close]

    If your get_future ndarray order is different, change row[2].
    """
    if isinstance(row, dict):
        return get_candle_value(row, ["low", "Low", "LowPrice"])

    if hasattr(row, "index"):
        return get_candle_value(row, ["low", "Low", "LowPrice"])

    return float(row[2])


def iter_future_rows(future_data):
    """
    Normalizes get_future output into iterable rows.

    Supports:
        - pandas DataFrame
        - numpy ndarray
        - list / tuple
    """

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


# =========================
# FIRST HIT LOGIC
# =========================
def direction_to_value(direction):
    if direction == "UP":
        return 1

    if direction == "DOWN":
        return -1

    return 0


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
        In this project, get_future(...) returns close-only numpy arrays,
        so first-hit should call this function with a df.loc[...] slice,
        not with get_future output.
    """

    if future_df is None or len(future_df) == 0:
        return {
            "Direction": "NOISE",
            "DirectionValue": 0,
            "HitIndex": None,
            "HitPrice": None,
            "HitPips": 0.0
        }

    target_price_distance = float(target_pips) * float(pip_size)

    up_price = float(entry_price) + target_price_distance
    down_price = float(entry_price) - target_price_distance

    # future_df should be a DataFrame slice with high/low columns.
    # Still supports list/dict/ndarray through iter_future_rows as fallback.
    for i, row in enumerate(iter_future_rows(future_df)):
        high = get_high_from_any_row(row)
        low = get_low_from_any_row(row)

        up_hit = high >= up_price
        down_hit = low <= down_price

        if up_hit and down_hit:
            if same_candle_policy == "up_first":
                return {
                    "Direction": "UP",
                    "DirectionValue": 1,
                    "HitIndex": int(i),
                    "HitPrice": float(up_price),
                    "HitPips": float(target_pips)
                }

            if same_candle_policy == "down_first":
                return {
                    "Direction": "DOWN",
                    "DirectionValue": -1,
                    "HitIndex": int(i),
                    "HitPrice": float(down_price),
                    "HitPips": float(-target_pips)
                }

            return {
                "Direction": "NOISE",
                "DirectionValue": 0,
                "HitIndex": int(i),
                "HitPrice": None,
                "HitPips": 0.0
            }

        if up_hit:
            return {
                "Direction": "UP",
                "DirectionValue": 1,
                "HitIndex": int(i),
                "HitPrice": float(up_price),
                "HitPips": float(target_pips)
            }

        if down_hit:
            return {
                "Direction": "DOWN",
                "DirectionValue": -1,
                "HitIndex": int(i),
                "HitPrice": float(down_price),
                "HitPips": float(-target_pips)
            }

    return {
        "Direction": "NOISE",
        "DirectionValue": 0,
        "HitIndex": None,
        "HitPrice": None,
        "HitPips": 0.0
    }

def compute_first_hit_for_tp(
    tp,
    df,
    index_map,
    timepoints,
    evaluation_config
):
    method = evaluation_config.get("future_direction_method", "return_sign")

    if method != "first_hit_pips":
        return {
            "Direction": None,
            "DirectionValue": None,
            "HitIndex": None,
            "HitPrice": None,
            "HitPips": None
        }

    if tp not in index_map:
        return {
            "Direction": "NOISE",
            "DirectionValue": 0,
            "HitIndex": None,
            "HitPrice": None,
            "HitPips": 0.0
        }

    try:
        entry_price = get_close_from_df(df, tp)
    except Exception:
        return {
            "Direction": "NOISE",
            "DirectionValue": 0,
            "HitIndex": None,
            "HitPrice": None,
            "HitPips": 0.0
        }

    current_index = index_map[tp]
    future_window_size = int(evaluation_config.get("first_hit_future_window", 10))

    start_index = current_index + 1
    end_index = min(current_index + 1 + future_window_size, len(timepoints))

    if start_index >= end_index:
        return {
            "Direction": "NOISE",
            "DirectionValue": 0,
            "HitIndex": None,
            "HitPrice": None,
            "HitPips": 0.0
        }

    # Faster than df.loc[future_tps] because we already have positional indexes
    future_df = df.iloc[start_index:end_index]

    return compute_first_hit_direction(
        future_df=future_df,
        entry_price=entry_price,
        target_pips=float(evaluation_config.get("first_hit_pips", 50)),
        pip_size=float(evaluation_config.get("pip_size", 0.01)),
        same_candle_policy=evaluation_config.get("same_candle_policy", "noise")
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
        "HistFirstHitUpRatio": float(up_ratio),
        "HistFirstHitDownRatio": float(down_ratio),
        "HistFirstHitNoiseRatio": float(noise_ratio),
        "HistFirstHitAgreement": float(agreement),

        "WeightedHistFirstHitUpRatio": float(weighted_up_ratio),
        "WeightedHistFirstHitDownRatio": float(weighted_down_ratio),
        "WeightedHistFirstHitAgreement": float(weighted_agreement),

        "HistFirstHitUpCount": up_count,
        "HistFirstHitDownCount": down_count,
        "HistFirstHitNoiseCount": noise_count,
        "HistFirstHitValidCount": valid_count,

        "PredFirstHitDirection": pred_direction,
        "PredFirstHitDirectionValue": pred_value,

        "WeightedPredFirstHitDirection": weighted_pred_direction,
        "WeightedPredFirstHitDirectionValue": weighted_pred_value,
    }


# =========================
# HISTORY RETURN CONSISTENCY
# =========================
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
    mean_abs_return = float(abs(mean_return))

    if std_return <= 1e-12:
        consistency_score = 0.0
    else:
        consistency_score = mean_abs_return / std_return

    return {
        "HistPositiveRatio": positive_ratio,
        "HistNegativeRatio": negative_ratio,
        "HistDirectionAgreement": direction_agreement,

        "WeightedHistPositiveRatio": weighted_positive_ratio,
        "WeightedHistNegativeRatio": weighted_negative_ratio,
        "WeightedHistDirectionAgreement": weighted_direction_agreement,

        "HistReturnMean": mean_return,
        "HistReturnMedian": median_return,
        "HistReturnStd": std_return,
        "HistConsistencyScore": consistency_score,
    }


# =========================
# BUILD DATASET BY CurrTP
# =========================
def build_dataset(matches_file, evaluation_config=None):

    print("\n=== BUILD DATASET BY CurrTP ===")

    if evaluation_config is None:
        evaluation_config = DEFAULT_EVALUATION_CONFIG

    if not os.path.exists(matches_file):
        raise FileNotFoundError(f"Matches file not found: {matches_file}")

    metadata, matches = load_matches(matches_file)

    candles_hist, _ = load_history_data()
    candles_curr, _ = load_current_data()

    hist_map = build_index_map(candles_hist)
    curr_map = build_index_map(candles_curr)

    df_hist = pd.DataFrame.from_dict(candles_hist, orient="index")
    df_curr = pd.DataFrame.from_dict(candles_curr, orient="index")

    hist_timepoints = list(df_hist.index)
    curr_timepoints = list(df_curr.index)

    hist_first_hit_cache = {}
    curr_first_hit_cache = {}

    rows = []

    use_first_hit = evaluation_config.get("future_direction_method") == "first_hit_pips"

    for i, (curr_tp_str, match_list) in enumerate(matches.items()):

        curr_tp = int(curr_tp_str)

        curr_future = get_future(curr_tp, df_curr, curr_map)
        curr_stats = compute_future_stats(curr_future)

        if curr_stats is None:
            continue

        if use_first_hit:
            if curr_tp not in curr_first_hit_cache:
                curr_first_hit_cache[curr_tp] = compute_first_hit_for_tp(
                    tp=curr_tp,
                    df=df_curr,
                    index_map=curr_map,
                    timepoints=curr_timepoints,
                    evaluation_config=evaluation_config
                )

            curr_first_hit = curr_first_hit_cache[curr_tp]
        else:
            curr_first_hit = None

        hist_returns = []
        hist_first_hit_values = []
        distances = []

        for m in match_list:

            match_tp = int(m["MatchTP"])
            distance = float(m["Distance"])

            hist_future = get_future(match_tp, df_hist, hist_map)
            hist_stats = compute_future_stats(hist_future)

            if hist_stats is None:
                continue

            hist_returns.append(float(hist_stats["Return"]))
            distances.append(distance)

            if use_first_hit:
                if match_tp not in hist_first_hit_cache:
                    hist_first_hit_cache[match_tp] = compute_first_hit_for_tp(
                        tp=match_tp,
                        df=df_hist,
                        index_map=hist_map,
                        timepoints=hist_timepoints,
                        evaluation_config=evaluation_config
                    )

                hist_first_hit = hist_first_hit_cache[match_tp]
                hist_first_hit_values.append(int(hist_first_hit["DirectionValue"]))

        if len(hist_returns) == 0:
            continue

        distances_arr = np.array(distances, dtype=np.float64)
        hist_returns_arr = np.array(hist_returns, dtype=np.float64)

        weights = 1.0 / (distances_arr + 1e-9)
        pred_return = np.average(hist_returns_arr, weights=weights)

        consistency = compute_history_consistency(
            hist_returns_arr=hist_returns_arr,
            weights_arr=weights
        )

        if consistency is None:
            continue

        pred_return_direction_value = 1 if pred_return > 0 else -1 if pred_return < 0 else 0

        row = {
            "CurrTP": curr_tp,
            "CurrReturn": float(curr_stats["Return"]),
            "PredReturn": float(pred_return),

            "PredReturnDirectionValue": int(pred_return_direction_value),

            "AvgHistReturn": float(np.mean(hist_returns_arr)),
            "MedianHistReturn": float(np.median(hist_returns_arr)),

            "MinDistance": float(np.min(distances_arr)),
            "AvgDistance": float(np.mean(distances_arr)),
            "MatchesCount": int(len(hist_returns)),

            "HistPositiveRatio": consistency["HistPositiveRatio"],
            "HistNegativeRatio": consistency["HistNegativeRatio"],
            "HistDirectionAgreement": consistency["HistDirectionAgreement"],

            "WeightedHistPositiveRatio": consistency["WeightedHistPositiveRatio"],
            "WeightedHistNegativeRatio": consistency["WeightedHistNegativeRatio"],
            "WeightedHistDirectionAgreement": consistency["WeightedHistDirectionAgreement"],

            "HistReturnMean": consistency["HistReturnMean"],
            "HistReturnMedian": consistency["HistReturnMedian"],
            "HistReturnStd": consistency["HistReturnStd"],
            "HistConsistencyScore": consistency["HistConsistencyScore"],
        }

        if use_first_hit:
            first_hit_consistency = compute_first_hit_consistency(
                direction_values_arr=np.array(hist_first_hit_values, dtype=np.int32),
                weights_arr=weights,
                ignore_noise=evaluation_config.get("ignore_noise_in_first_hit_prediction", True)
            )

            if first_hit_consistency is None:
                continue

            row.update({
                "CurrFirstHitDirection": curr_first_hit["Direction"],
                "CurrFirstHitDirectionValue": int(curr_first_hit["DirectionValue"]),
                "CurrFirstHitIndex": curr_first_hit["HitIndex"],
                "CurrFirstHitPips": curr_first_hit["HitPips"],

                "PredFirstHitDirection": first_hit_consistency["PredFirstHitDirection"],
                "PredFirstHitDirectionValue": int(first_hit_consistency["PredFirstHitDirectionValue"]),

                "WeightedPredFirstHitDirection": first_hit_consistency["WeightedPredFirstHitDirection"],
                "WeightedPredFirstHitDirectionValue": int(first_hit_consistency["WeightedPredFirstHitDirectionValue"]),

                "HistFirstHitUpRatio": first_hit_consistency["HistFirstHitUpRatio"],
                "HistFirstHitDownRatio": first_hit_consistency["HistFirstHitDownRatio"],
                "HistFirstHitNoiseRatio": first_hit_consistency["HistFirstHitNoiseRatio"],
                "HistFirstHitAgreement": first_hit_consistency["HistFirstHitAgreement"],

                "WeightedHistFirstHitUpRatio": first_hit_consistency["WeightedHistFirstHitUpRatio"],
                "WeightedHistFirstHitDownRatio": first_hit_consistency["WeightedHistFirstHitDownRatio"],
                "WeightedHistFirstHitAgreement": first_hit_consistency["WeightedHistFirstHitAgreement"],

                "HistFirstHitUpCount": first_hit_consistency["HistFirstHitUpCount"],
                "HistFirstHitDownCount": first_hit_consistency["HistFirstHitDownCount"],
                "HistFirstHitNoiseCount": first_hit_consistency["HistFirstHitNoiseCount"],
                "HistFirstHitValidCount": first_hit_consistency["HistFirstHitValidCount"],
            })

        rows.append(row)

        if i % 200 == 0:
            print(f"Processed {i}")

    df = pd.DataFrame(rows)

    print(f"\nDataset built before filters: {len(df)} current patterns")

    return df, metadata


# =========================
# FILTER DATASET BY CONFIG
# =========================
def filter_dataset_by_evaluation_config(df, evaluation_config):
    if df.empty:
        return df

    df = df.copy()

    print("\n=== APPLY CONSISTENCY FILTERS ===")

    before_all = len(df)

    min_direction_agreement = evaluation_config.get("min_direction_agreement")
    min_weighted_direction_agreement = evaluation_config.get("min_weighted_direction_agreement")
    min_consistency_score = evaluation_config.get("min_consistency_score")
    max_return_std = evaluation_config.get("max_return_std")
    min_matches_count = evaluation_config.get("min_matches_count")

    min_first_hit_agreement = evaluation_config.get("min_first_hit_agreement")
    min_weighted_first_hit_agreement = evaluation_config.get("min_weighted_first_hit_agreement")
    min_valid_first_hit_count = evaluation_config.get("min_valid_first_hit_count")
    max_first_hit_noise_ratio = evaluation_config.get("max_first_hit_noise_ratio")

    if min_matches_count is not None:
        before = len(df)
        df = df[df["MatchesCount"] >= int(min_matches_count)]
        print(f"Filter MatchesCount >= {min_matches_count}: {before} -> {len(df)}")

    if min_direction_agreement is not None:
        before = len(df)
        df = df[df["HistDirectionAgreement"] >= float(min_direction_agreement)]
        print(f"Filter HistDirectionAgreement >= {min_direction_agreement}: {before} -> {len(df)}")

    if min_weighted_direction_agreement is not None:
        before = len(df)
        df = df[df["WeightedHistDirectionAgreement"] >= float(min_weighted_direction_agreement)]
        print(
            f"Filter WeightedHistDirectionAgreement >= {min_weighted_direction_agreement}: "
            f"{before} -> {len(df)}"
        )

    if min_consistency_score is not None:
        before = len(df)
        df = df[df["HistConsistencyScore"] >= float(min_consistency_score)]
        print(f"Filter HistConsistencyScore >= {min_consistency_score}: {before} -> {len(df)}")

    if max_return_std is not None:
        before = len(df)
        df = df[df["HistReturnStd"] <= float(max_return_std)]
        print(f"Filter HistReturnStd <= {max_return_std}: {before} -> {len(df)}")

    if "HistFirstHitAgreement" in df.columns and min_first_hit_agreement is not None:
        before = len(df)
        df = df[df["HistFirstHitAgreement"] >= float(min_first_hit_agreement)]
        print(f"Filter HistFirstHitAgreement >= {min_first_hit_agreement}: {before} -> {len(df)}")

    if "WeightedHistFirstHitAgreement" in df.columns and min_weighted_first_hit_agreement is not None:
        before = len(df)
        df = df[df["WeightedHistFirstHitAgreement"] >= float(min_weighted_first_hit_agreement)]
        print(
            f"Filter WeightedHistFirstHitAgreement >= {min_weighted_first_hit_agreement}: "
            f"{before} -> {len(df)}"
        )

    if "HistFirstHitValidCount" in df.columns and min_valid_first_hit_count is not None:
        before = len(df)
        df = df[df["HistFirstHitValidCount"] >= int(min_valid_first_hit_count)]
        print(f"Filter HistFirstHitValidCount >= {min_valid_first_hit_count}: {before} -> {len(df)}")

    if "HistFirstHitNoiseRatio" in df.columns and max_first_hit_noise_ratio is not None:
        before = len(df)
        df = df[df["HistFirstHitNoiseRatio"] <= float(max_first_hit_noise_ratio)]
        print(f"Filter HistFirstHitNoiseRatio <= {max_first_hit_noise_ratio}: {before} -> {len(df)}")

    print(f"Total after filters: {before_all} -> {len(df)}")

    return df


# =========================
# ACCURACY HELPERS
# =========================
def get_accuracy_columns(evaluation_config):
    method = evaluation_config.get("future_direction_method", "return_sign")

    if method == "first_hit_pips":
        pred_col = "PredFirstHitDirectionValue"
        real_col = "CurrFirstHitDirectionValue"

        if evaluation_config.get("use_weighted_first_hit_prediction", False):
            pred_col = "WeightedPredFirstHitDirectionValue"

        return pred_col, real_col

    return "PredReturnDirectionValue", "CurrReturnDirectionValue"


def add_return_direction_columns(df):
    df = df.copy()

    df["CurrReturnDirectionValue"] = np.where(
        df["CurrReturn"] > 0,
        1,
        np.where(df["CurrReturn"] < 0, -1, 0)
    )

    df["PredReturnDirectionValue"] = np.where(
        df["PredReturn"] > 0,
        1,
        np.where(df["PredReturn"] < 0, -1, 0)
    )

    return df


def calc_direction_accuracy(df, evaluation_config):
    if df.empty:
        return None

    df = add_return_direction_columns(df)

    pred_col, real_col = get_accuracy_columns(evaluation_config)

    if pred_col not in df.columns or real_col not in df.columns:
        return None

    eval_df = df.copy()

    if evaluation_config.get("future_direction_method") == "first_hit_pips":
        if not evaluation_config.get("include_noise_in_accuracy", False):
            eval_df = eval_df[
                (eval_df[pred_col] != 0) &
                (eval_df[real_col] != 0)
            ]

    if len(eval_df) == 0:
        return None

    return float((eval_df[pred_col] == eval_df[real_col]).mean())


# =========================
# DEBUG ANALYSIS
# =========================
def debug_checks(df, evaluation_config):

    print("\n=== DEBUG BY CurrTP ===")

    df = add_return_direction_columns(df)

    corr = df[[
        "PredReturn",
        "AvgHistReturn",
        "MedianHistReturn",
        "CurrReturn"
    ]].corr()

    print("\nPrediction correlation:")
    print(corr)

    direction_accuracy = calc_direction_accuracy(df, evaluation_config)

    print("\nDirection Accuracy:")
    print(direction_accuracy)

    pred_up_df = df[df["PredReturn"] > 0]
    pred_down_df = df[df["PredReturn"] < 0]

    avg_return_pred_up = float(pred_up_df["CurrReturn"].mean()) if len(pred_up_df) else None
    avg_return_pred_down = float(pred_down_df["CurrReturn"].mean()) if len(pred_down_df) else None

    print("\nAvg Return when PredReturn > 0:")
    print(avg_return_pred_up)

    print("\nAvg Return when PredReturn < 0:")
    print(avg_return_pred_down)

    print("\nHistorical return agreement summary:")
    if "HistDirectionAgreement" in df.columns:
        print(df[[
            "HistDirectionAgreement",
            "WeightedHistDirectionAgreement",
            "HistConsistencyScore",
            "HistReturnStd",
            "MatchesCount"
        ]].describe())

    if "HistFirstHitAgreement" in df.columns:
        print("\nHistorical first-hit summary:")
        print(df[[
            "HistFirstHitAgreement",
            "WeightedHistFirstHitAgreement",
            "HistFirstHitNoiseRatio",
            "HistFirstHitValidCount"
        ]].describe())

        print("\nCurrent first-hit distribution:")
        print(df["CurrFirstHitDirection"].value_counts(dropna=False))

        print("\nPred first-hit distribution:")
        print(df["PredFirstHitDirection"].value_counts(dropna=False))

    metrics = {
        "PredReturnCorr": safe_corr_value(corr, "PredReturn", "CurrReturn"),
        "AvgHistReturnCorr": safe_corr_value(corr, "AvgHistReturn", "CurrReturn"),
        "MedianHistReturnCorr": safe_corr_value(corr, "MedianHistReturn", "CurrReturn"),
        "DirectionAccuracy": direction_accuracy,
        "AvgReturnPredUp": avg_return_pred_up,
        "AvgReturnPredDown": avg_return_pred_down,
    }

    return metrics


def safe_corr_value(corr_df, row, col):
    try:
        value = corr_df.loc[row, col]

        if pd.isna(value):
            return None

        return float(value)

    except Exception:
        return None


# =========================
# EDGE FINDER BY CONFIDENCE
# =========================
def find_edge(df, evaluation_config):

    print("\n=== EDGE FINDER BY CONFIDENCE ===")

    if df.empty:
        raise ValueError("Dataset is empty after filters ❌")

    df = add_return_direction_columns(df)

    baseline = float(df["CurrReturn"].mean())
    print(f"Baseline Current Return: {baseline:.8f}")

    df["PredAbs"] = df["PredReturn"].abs()

    pred_col, real_col = get_accuracy_columns(evaluation_config)

    levels = [
        ("Top50", 0.50),
        ("Top40", 0.60),
        ("Top30", 0.70),
        ("Top20", 0.80),
        ("Top10", 0.90),
        ("Top5", 0.95),
    ]

    results = []

    for label, q in levels:

        threshold = float(df["PredAbs"].quantile(q))
        subset = df[df["PredAbs"] >= threshold].copy()

        if evaluation_config.get("future_direction_method") == "first_hit_pips":
            if not evaluation_config.get("include_noise_in_accuracy", False):
                subset = subset[
                    (subset[pred_col] != 0) &
                    (subset[real_col] != 0)
                ]

        if len(subset) < 100:
            continue

        acc = float((subset[pred_col] == subset[real_col]).mean())
        avg_ret = float(subset["CurrReturn"].mean())

        results.append({
            "Label": label,
            "Threshold": threshold,
            "Samples": int(len(subset)),
            "DirectionAccuracy": acc,
            "AvgCurrReturn": avg_ret,
            "Edge": avg_ret - baseline
        })

    results_df = pd.DataFrame(results)

    print("\n=== TOP CONFIDENCE RESULTS ===")
    print(results_df)

    return results_df, baseline


# =========================
# GET EDGE VALUE BY LABEL
# =========================
def get_edge_metric(edge_df, label, field):

    if edge_df.empty:
        return None

    row = edge_df[edge_df["Label"] == label]

    if row.empty:
        return None

    value = row.iloc[0][field]

    if pd.isna(value):
        return None

    if field == "Samples":
        return int(value)

    return float(value)


# =========================
# MAIN CALLABLE FUNCTION
# =========================
def run_edge_for_match_run(
    match_run_id,
    matches_file=None,
    evaluation_config=None
):
    if evaluation_config is None:
        evaluation_config = DEFAULT_EVALUATION_CONFIG

    if matches_file is None:
        matches_file = get_match_run_file_from_db(match_run_id)

    print(f"\n=== EDGE FINDER FOR MatchRunID={match_run_id} ===")
    print(f"Matches file: {matches_file}")
    print(f"Evaluation config: {evaluation_config}")

    try:
        df, metadata = build_dataset(
            matches_file=matches_file,
            evaluation_config=evaluation_config
        )

        if len(df) == 0:
            raise ValueError("Dataset is empty ❌")

        df = filter_dataset_by_evaluation_config(
            df=df,
            evaluation_config=evaluation_config
        )

        if len(df) == 0:
            raise ValueError("Dataset is empty after consistency filters ❌")

        debug_metrics = debug_checks(
            df=df,
            evaluation_config=evaluation_config
        )

        edge_df, baseline_return = find_edge(
            df=df,
            evaluation_config=evaluation_config
        )

        edge_run_id = insert_edge_run_to_db(
            match_run_id=match_run_id,
            evaluation_config=evaluation_config,
            debug_metrics=debug_metrics,
            edge_df=edge_df,
            baseline_return=baseline_return,
            dataset_rows=len(df)
        )

        return {
            "match_run_id": int(match_run_id),
            "edge_run_id": int(edge_run_id),
            "matches_file": matches_file,
            "dataset_rows": int(len(df)),
            "debug_metrics": debug_metrics,
            "edge_results": edge_df.to_dict(orient="records")
        }

    except Exception as e:
        print("❌ Edge finder failed")
        print(traceback.format_exc())
        raise e


# =========================
# MANUAL RUN
# =========================
def run():
    return run_edge_for_match_run(
        match_run_id=MATCH_RUN_ID,
        matches_file=f"{MATCHES_DIR}/matches_run_{MATCH_RUN_ID}.json",
        evaluation_config=DEFAULT_EVALUATION_CONFIG
    )


if __name__ == "__main__":
    run()