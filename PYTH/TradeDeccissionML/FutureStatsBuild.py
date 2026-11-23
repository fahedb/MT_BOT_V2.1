import os
import numpy as np
import pandas as pd
from sqlalchemy import create_engine, text

# =========================
# CONFIG
# =========================
TP_PIPS = 200
SL_PIPS = 200
PIP = 0.01
C = 10

SPREAD_PIPS = 2   # 🔥 תכוון לפי הנכס
SPREAD = SPREAD_PIPS * PIP

# Connection string: override with the MTBOT_SQL_CONN environment variable.
# See docs/CONFIGURATION.md (refactor plan Phase 0, finding E12).
CONN_STR = os.environ.get(
    "MTBOT_SQL_CONN",
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes",
)

# =========================
# CORE FUNCTION
# =========================
def compute_future_stats(simulation_id: int):

    print(f"\n=== Computing FutureStats | SimulationID={simulation_id} ===")

    engine = create_engine(CONN_STR, fast_executemany=True)

    # =========================
    # Load candles
    # =========================
    df = pd.read_sql("""
        SELECT TimePoint, CoinsPair, HighPrice, LowPrice, ClosePrice
        FROM dbo.CurrentTimeLine
        WHERE CoinsPair = 'XAUUSD'
        ORDER BY TimePoint
    """, engine)

    print(f"Loaded candles: {len(df)}")

    # =========================
    # Convert to numpy
    # =========================
    time_arr  = df["TimePoint"].values
    pair_arr  = df["CoinsPair"].values
    high_arr  = df["HighPrice"].values
    low_arr   = df["LowPrice"].values
    close_arr = df["ClosePrice"].values

    n = len(df)

    tp_hit = np.full(n, None)
    sl_hit = np.full(n, None)
    is_win = np.zeros(n)
    first_hit_type = np.full(n, None)

    ambiguous_count = 0  # 🔥 כמה פעמים TP+SL קרו יחד

    # =========================
    # MAIN LOOP
    # =========================
    for i in range(n):

        entry = close_arr[i]

        # 🔥 TP/SL עם spread
        tp = entry + TP_PIPS * PIP
        sl = entry - SL_PIPS * PIP - SPREAD

        for j in range(1, C + 1):
            if i + j >= n:
                break

            high  = high_arr[i + j]
            low   = low_arr[i + j]
            close = close_arr[i + j]

            hit_tp = high >= tp
            hit_sl = low <= sl

            # =========================
            # 🎯 Case 1: TP + SL באותו נר
            # =========================
            if hit_tp and hit_sl:
                ambiguous_count += 1

                # הכרעה לפי כיוון הנר
                if close >= entry:
                    tp_hit[i] = j
                    is_win[i] = 1
                    first_hit_type[i] = 1
                else:
                    sl_hit[i] = j
                    first_hit_type[i] = 0

                break

            # =========================
            # 🎯 Case 2: רק TP
            # =========================
            elif hit_tp:
                tp_hit[i] = j
                is_win[i] = 1
                first_hit_type[i] = 1
                break

            # =========================
            # 🎯 Case 3: רק SL
            # =========================
            elif hit_sl:
                sl_hit[i] = j
                first_hit_type[i] = 0
                break

        if i % 50000 == 0 and i > 0:
            print(f"Processed {i}/{n}")

    print("Computation done ✔")
    print(f"Ambiguous candles (TP+SL same candle): {ambiguous_count}")

    # =========================
    # Build result DF
    # =========================
    result = pd.DataFrame({
        "SimulationID": simulation_id,
        "TimePoint": time_arr,
        "CoinsPair": pair_arr,
        "TP_HitIndex": tp_hit,
        "SL_HitIndex": sl_hit,
        "IsWin": is_win,
        "FirstHitType": first_hit_type
    })

    # =========================
    # Insert to SQL
    # =========================
    result.to_sql(
        "FutureStats",
        engine,
        if_exists="append",
        index=False,
        chunksize=20000
    )

    print("Inserted into FutureStats ✔")

    # =========================
    # Index (אם לא קיים)
    # =========================
    with engine.begin() as conn:
        conn.execute(text("""
            IF NOT EXISTS (
                SELECT * FROM sys.indexes 
                WHERE name = 'IX_FutureStats_Sim_Time'
            )
            CREATE INDEX IX_FutureStats_Sim_Time
            ON dbo.FutureStats (SimulationID, TimePoint)
        """))

    print("Index ensured ✔")