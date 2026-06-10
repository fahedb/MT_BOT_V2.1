import json
import os
import pickle

import pandas as pd
from sqlalchemy import create_engine


# =========================
# CONFIG
# =========================

CACHE_DIR = "Cache"

CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

PAIR = "XAUUSD"

TIMEFRAMES = ["M1", "M15", "H1"]
LENS = [10, 20, 30]
INDICATORS = ["RSI", "EMA"]

# =========================
# DATA SOURCE CONFIG
# =========================

# Options:
# "sql"  = load from SQL tables
# "json" = load from JSONL file
CURRENT_SOURCE = "json"

# JSONL file path.
# Each line should be a JSON object with one TimePoint key.
CURRENT_JSON_PATH = r"C:\\MTDB\\BackTestCurrData\\XAUUSD_M1_DATA.json"

# If your JSON TimePoint is like 20260414225200000
# and your system expects this exact format, keep False.
# If you need to convert it to 20260414225200, set True.
TRIM_JSON_TIMEPOINT_MILLISECONDS = False

# Separate cache per source, so SQL cache and JSON cache do not collide.
CACHE_PATH_SQL = os.path.join(CACHE_DIR, "current_loader_sql_cache.pkl")
CACHE_PATH_JSON = os.path.join(CACHE_DIR, "current_loader_json_cache.pkl")


# =========================
# HELPERS
# =========================

def get_cache_path():
    if CURRENT_SOURCE == "sql":
        return CACHE_PATH_SQL

    if CURRENT_SOURCE == "json":
        return CACHE_PATH_JSON

    raise ValueError(f"Unsupported CURRENT_SOURCE: {CURRENT_SOURCE}")


def safe_float(value):
    if value is None:
        return None

    return float(value)


def normalize_timepoint(timepoint):
    tp = int(timepoint)

    if TRIM_JSON_TIMEPOINT_MILLISECONDS:
        # Example:
        # 20260414225200000 -> 20260414225200
        tp = tp // 1000

    return tp


# =========================
# LOAD CANDLES FROM SQL
# =========================

def load_candles_from_sql():
    print("\n=== Loading CURRENT candles from SQL ===")

    engine = create_engine(CONN_STR)

    query = f"""
    SELECT TimePoint, ClosePrice, HighPrice, LowPrice, OpenPrice
    FROM dbo.CurrentTimeLine
    WHERE CoinsPair = '{PAIR}'
    ORDER BY TimePoint
    """

    df = pd.read_sql(query, engine)

    if df.empty:
        raise ValueError("No candles found ❌")

    candles = {}

    for _, row in df.iterrows():
        tp = int(row["TimePoint"])

        candles[tp] = {
            "close": float(row["ClosePrice"]),
            "high": float(row["HighPrice"]),
            "low": float(row["LowPrice"]),
            "open": float(row["OpenPrice"]),
        }

    print(f"Candles loaded: {len(candles)}")

    return candles


# =========================
# LOAD INDICATORS FROM SQL
# =========================

def load_indicators_from_sql():
    print("\n=== Loading CURRENT indicators from SQL ===")

    engine = create_engine(CONN_STR)

    query = f"""
    SELECT 
        TimePoint,
        IndicatorName,
        TimeFrame,
        IndLen,
        IndicatorValue,
        IndicatorSlope
    FROM dbo.CurrentIndicatorTimeLine
    WHERE CoinsPair = '{PAIR}'
      AND IndicatorName IN ('RSI', 'EMA')
    ORDER BY TimePoint
    """

    df = pd.read_sql(query, engine)

    if df.empty:
        raise ValueError("No indicators found ❌")

    indicators = {}

    for _, row in df.iterrows():
        tp = int(row["TimePoint"])
        ind = row["IndicatorName"]
        tf = row["TimeFrame"]
        ln = int(row["IndLen"])

        if ind not in INDICATORS:
            continue

        if tf not in TIMEFRAMES:
            continue

        if ln not in LENS:
            continue

        indicators.setdefault(tp, {}) \
            .setdefault(ind, {}) \
            .setdefault(tf, {})[ln] = {
                "value": float(row["IndicatorValue"]),
                "slope": float(row["IndicatorSlope"]),
            }

    print(f"Indicator rows loaded: {len(df)}")
    print(f"Unique TimePoints: {len(indicators)}")

    return indicators


# =========================
# LOAD FROM JSONL FILE
# =========================

def load_current_from_json_file(json_path):
    print("\n=== Loading CURRENT from JSONL file ===")
    print(f"File: {json_path}")

    if not os.path.exists(json_path):
        raise FileNotFoundError(f"JSON file not found: {json_path}")

    candles = {}
    indicators = {}

    total_lines = 0
    loaded_rows = 0
    skipped_empty_lines = 0
    skipped_invalid_lines = 0

    with open(json_path, "r", encoding="utf-8") as f:
        for line in f:
            total_lines += 1

            line = line.strip()

            if not line:
                skipped_empty_lines += 1
                continue

            try:
                line_obj = json.loads(line)
            except json.JSONDecodeError as ex:
                skipped_invalid_lines += 1
                print(f"⚠️ Invalid JSON line {total_lines}: {ex}")
                continue

            if not isinstance(line_obj, dict):
                skipped_invalid_lines += 1
                print(f"⚠️ Line {total_lines} is not a JSON object")
                continue

            # Each line should contain one TimePoint key.
            for raw_tp, row in line_obj.items():
                tp = normalize_timepoint(raw_tp)

                candles[tp] = {
                    "close": safe_float(row.get("ClosePrice")),
                    "high": safe_float(row.get("HighPrice")),
                    "low": safe_float(row.get("LowPrice")),
                    "open": safe_float(row.get("OpenPrice")),
                }

                row_indicators = row.get("indicators", {})

                for ind in INDICATORS:
                    ind_obj = row_indicators.get(ind)

                    if not ind_obj:
                        continue

                    for ln in LENS:
                        len_key = f"L{ln}"
                        len_obj = ind_obj.get(len_key)

                        if not len_obj:
                            continue

                        values_obj = len_obj.get("Values", {})
                        slopes_obj = len_obj.get("Slopes", {})

                        for tf in TIMEFRAMES:
                            if tf not in values_obj or tf not in slopes_obj:
                                continue

                            indicators.setdefault(tp, {}) \
                                .setdefault(ind, {}) \
                                .setdefault(tf, {})[ln] = {
                                    "value": safe_float(values_obj[tf]),
                                    "slope": safe_float(slopes_obj[tf]),
                                }

                loaded_rows += 1

    if not candles:
        raise ValueError("No candles loaded from JSON file ❌")

    print(f"Total lines read: {total_lines}")
    print(f"Rows loaded: {loaded_rows}")
    print(f"Skipped empty lines: {skipped_empty_lines}")
    print(f"Skipped invalid lines: {skipped_invalid_lines}")
    print(f"Candles loaded: {len(candles)}")
    print(f"Unique indicator TimePoints: {len(indicators)}")

    return candles, indicators


# =========================
# LOAD BY SOURCE
# =========================

def load_current_from_sql():
    candles = load_candles_from_sql()
    indicators = load_indicators_from_sql()

    return candles, indicators


def load_current_data():
    if CURRENT_SOURCE == "sql":
        return load_current_from_sql()

    if CURRENT_SOURCE == "json":
        return load_current_from_json_file(CURRENT_JSON_PATH)

    raise ValueError(f"Unsupported CURRENT_SOURCE: {CURRENT_SOURCE}")


# =========================
# VALIDATION
# =========================

def validate_data(candles, indicators):
    print("\n=== Validating CURRENT data ===")

    missing_timepoints = 0
    missing_values = 0

    for tp in candles:
        if tp not in indicators:
            missing_timepoints += 1
            raise ValueError(f"❌ Missing indicators for TimePoint: {tp}")

        for ind in INDICATORS:
            for tf in TIMEFRAMES:
                for ln in LENS:
                    try:
                        item = indicators[tp][ind][tf][ln]
                    except KeyError:
                        missing_values += 1
                        raise ValueError(
                            f"❌ Missing data → TP={tp} | {ind} {tf} L{ln}"
                        )

                    if item["value"] is None or item["slope"] is None:
                        raise ValueError(
                            f"❌ Null indicator value → TP={tp} | {ind} {tf} L{ln}"
                        )

    print("✅ Validation passed")
    print(f"Validated candles: {len(candles)}")


# =========================
# MAIN
# =========================

def load_history(use_cache=True, force_reload=False):
    """
    Keeps the old function name for compatibility.
    This loader returns CURRENT data:
    candles, indicators
    """

    cache_path = get_cache_path()

    if use_cache and not force_reload and os.path.exists(cache_path):
        print("\n=== Loading CURRENT from cache ===")
        print(f"Cache: {cache_path}")

        with open(cache_path, "rb") as f:
            return pickle.load(f)

    candles, indicators = load_current_data()

    validate_data(candles, indicators)

    data = (candles, indicators)

    if use_cache:
        os.makedirs(CACHE_DIR, exist_ok=True)

        with open(cache_path, "wb") as f:
            pickle.dump(data, f, protocol=pickle.HIGHEST_PROTOCOL)

        print("✅ Saved CURRENT cache")
        print(f"Cache: {cache_path}")

    return data


# =========================
# DEBUG
# =========================

if __name__ == "__main__":
    candles, indicators = load_history(
        use_cache=True,
        force_reload=True
    )

    print("\nSample candle:")
    print(list(candles.items())[:1])

    print("\nSample indicator:")
    print(list(indicators.items())[:1])