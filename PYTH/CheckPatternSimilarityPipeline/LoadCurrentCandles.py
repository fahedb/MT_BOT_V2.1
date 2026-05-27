import pandas as pd
from sqlalchemy import create_engine

# =========================
# CONFIG
# =========================
CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

DEFAULT_PAIR = "XAUUSD"


# =========================
# LOAD CURRENT
# =========================
def load_current_candles(pair=DEFAULT_PAIR):

    print("\n=== Loading CURRENT candles ===")

    engine = create_engine(CONN_STR)

    query = f"""
    SELECT TimePoint, ClosePrice
    FROM dbo.CurrentTimeLine
    WHERE CoinsPair = '{pair}'
    ORDER BY TimePoint
    """
    
    df = pd.read_sql(query, engine)
    
    if df.empty:
        raise ValueError("No CURRENT candles found ❌")

    df = df.reset_index(drop=True)

    time_to_index = dict(zip(df["TimePoint"], df.index))

    print(f"Loaded CURRENT candles: {len(df)}")

    return df, time_to_index