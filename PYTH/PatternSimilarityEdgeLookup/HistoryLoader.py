import os
import pandas as pd
from sqlalchemy import create_engine

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

PAIR = "XAUUSD"

TIMEFRAMES = ["M1", "M15", "H1", "H4"]
LENS = [10, 20, 30]

INDICATORS = ["RSI", "EMA"]


# =========================
# LOAD CANDLES
# =========================
def load_candles():

    print("\n=== Loading HISTORY candles ===")

    engine = create_engine(CONN_STR)

    query = f"""
    SELECT TimePoint, ClosePrice, HighPrice, LowPrice
    FROM dbo.HistoryTimeLine
    WHERE CoinsPair = '{PAIR}'
    ORDER BY TimePoint
    """

    df = pd.read_sql(query, engine)

    if df.empty:
        raise ValueError("No candles found ❌")

    candles = {}

    for _, row in df.iterrows():
        candles[int(row["TimePoint"])] = {
            "close": float(row["ClosePrice"]),
            "high" : float(row["HighPrice"]),
            "low" : float(row["LowPrice"])
        }

    print(f"Candles loaded: {len(candles)}")

    return candles


# =========================
# LOAD INDICATORS
# =========================
def load_indicators():

    print("\n=== Loading HISTORY indicators ===")

    engine = create_engine(CONN_STR)

    query = f"""
    SELECT 
        TimePoint,
        IndicatorName,
        TimeFrame,
        IndLen,
        IndicatorValue,
        IndicatorSlope
    FROM dbo.HistoryIndicatorTimeLine
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
                "slope": float(row["IndicatorSlope"])
            }

    print(f"Indicator rows loaded: {len(df)}")
    print(f"Unique TimePoints: {len(indicators)}")

    return indicators


# =========================
# VALIDATION
# =========================
def validate_data(candles, indicators):

    print("\n=== Validating data ===")

    for tp in candles:

        if tp not in indicators:
            raise ValueError(f"❌ Missing indicators for TimePoint: {tp}")

        for ind in INDICATORS:
            for tf in TIMEFRAMES:
                for ln in LENS:

                    try:
                        indicators[tp][ind][tf][ln]
                    except KeyError:
                        raise ValueError(
                            f"❌ Missing data → TP={tp} | {ind} {tf} L{ln}"
                        )

    print("✅ Validation passed")


# =========================
# MAIN
# =========================
def load_history():

    candles = load_candles()
    indicators = load_indicators()

    validate_data(candles, indicators)

    return candles, indicators


# =========================
# DEBUG
# =========================
if __name__ == "__main__":

    candles, indicators = load_history()

    print("\nSample candle:")
    print(list(candles.items())[:1])

    print("\nSample indicator:")
    print(list(indicators.items())[:1])