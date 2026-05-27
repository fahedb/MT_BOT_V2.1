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
# LOAD FUNCTION
# =========================
def load_candles(pair=DEFAULT_PAIR):

    print("\n=== Loading candles from DB ===")
    print(f"Pair: {pair}")

    engine = create_engine(CONN_STR)

    query = f"""
    SELECT TimePoint, ClosePrice
    FROM dbo.HistoryTimeLine
    WHERE CoinsPair = '{pair}'
    ORDER BY TimePoint
    """
    
    df = pd.read_sql(query, engine)

    if df.empty:
        raise ValueError("No candles found ❌")

    # =========================
    # SORT + INDEX RESET
    # =========================
    df = df.sort_values("TimePoint").reset_index(drop=True)

    # =========================
    # BUILD FAST LOOKUP
    # =========================
    time_to_index = dict(zip(df["TimePoint"], df.index))

    print(f"Total candles loaded: {len(df)}")

    return df, time_to_index


# =========================
# OPTIONAL: GET INDEX SAFE
# =========================
def get_index(time_to_index, tp):
    return time_to_index.get(tp, None)


# =========================
# OPTIONAL: GET PRICE SAFE
# =========================
def get_close(df, idx):
    if idx < 0 or idx >= len(df):
        return None
    return df.iloc[idx]["ClosePrice"]


# =========================
# DEBUG ENTRY (standalone run)
# =========================
if __name__ == "__main__":

    df, mapping = load_candles()

    print("\nSample:")
    print(df.head())

    print("\nMapping sample:")
    for i, (k, v) in enumerate(mapping.items()):
        if i >= 5:
            break
        print(k, "->", v)