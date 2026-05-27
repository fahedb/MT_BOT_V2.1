import numpy as np
import pandas as pd
from sqlalchemy import create_engine

# =========================
# CONFIG
# =========================
CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

COIN = "XAUUSD"
FUTURE_WINDOW = 10   # 🔥 כמה נרות קדימה לבדוק (תשחק עם זה)

# =========================
# MAIN FUNCTION
# =========================
def compute_future_distribution():

    print("\n=== Computing Future Distribution ===")

    engine = create_engine(CONN_STR)

    # =========================
    # Load candles
    # =========================
    df = pd.read_sql(f"""
        SELECT TimePoint, HighPrice, LowPrice, ClosePrice
        FROM dbo.CurrentTimeLine
        WHERE CoinsPair = '{COIN}'
        ORDER BY TimePoint
    """, engine)

    print(f"Loaded candles: {len(df)}")

    # =========================
    # Convert to numpy (fast)
    # =========================
    high_arr = df["HighPrice"].values
    low_arr = df["LowPrice"].values
    close_arr = df["ClosePrice"].values

    n = len(df)

    max_up = np.zeros(n)
    max_down = np.zeros(n)

    # =========================
    # MAIN LOOP
    # =========================
    for i in range(n):

        entry = close_arr[i]

        future_high = entry
        future_low = entry

        for j in range(1, FUTURE_WINDOW + 1):
            if i + j >= n:
                break

            # update extremes
            if high_arr[i + j] > future_high:
                future_high = high_arr[i + j]

            if low_arr[i + j] < future_low:
                future_low = low_arr[i + j]

        max_up[i] = future_high - entry
        max_down[i] = entry - future_low

        if i % 50000 == 0 and i > 0:
            print(f"Processed {i}/{n}")

    print("Computation done ✔")

    # =========================
    # Build DataFrame
    # =========================
    result = pd.DataFrame({
        "MaxUp": max_up,
        "MaxDown": max_down
    })

    return result


# =========================
# ANALYSIS FUNCTION
# =========================
def analyze_distribution(df):

    print("\n=== ANALYSIS ===")

    avg_up = df["MaxUp"].mean()
    avg_down = df["MaxDown"].mean()

    up_stronger = (df["MaxUp"] > df["MaxDown"]).mean()

    print(f"Avg Up Move:   {avg_up:.5f}")
    print(f"Avg Down Move: {avg_down:.5f}")
    print(f"Up stronger %: {up_stronger:.4f}")

    print("\n=== INTERPRETATION ===")

    if abs(up_stronger - 0.5) < 0.01:
        print("➡️ No edge (market is symmetric)")
    elif up_stronger > 0.5:
        print("➡️ Bullish bias detected")
    else:
        print("➡️ Bearish bias detected")


# =========================
# ENTRY POINT
# =========================
if __name__ == "__main__":
    print("Start")
    df = compute_future_distribution()
    print("Done Dist")
    analyze_distribution(df)