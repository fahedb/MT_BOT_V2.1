import os
import pandas as pd
from sqlalchemy import create_engine, text
from pathlib import Path

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

OUTPUT_DIR = Path(r"C:\\MT_Simulator_Exports\\ML_DATA")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)


# =========================
# MAIN FUNCTION
# =========================
def build_dataset(simulation_id: int, stat_type: str):

    print(f"\n=== Building dataset | {stat_type} | SimulationID={simulation_id} ===")

    engine = create_engine(CONN_STR)

    query = text("""
    SELECT
        d.TimePoint,

        -- RAW FEATURES
        d.Mean,
        d.PureMean_Up,
        d.PureMean_Down,
        d.UpPercentage,
        d.DownPercentage,
        d.TotalVotes,
        d.StdDev,

        -- TARGET
        f.IsWin

    FROM dbo.BackTest_DirectionStats d

    JOIN dbo.FutureStats f
        ON f.TimePoint = d.TimePoint
       AND f.CoinsPair = d.CoinsPair
       AND f.SimulationID = d.SimulationID

    WHERE d.SimulationID = :sid
      AND d.VoteType = :stype
      AND f.IsWin IS NOT NULL
      AND d.TotalVotes >= 2

    ORDER BY d.TimePoint
    """)

    df = pd.read_sql(query, engine, params={
        "sid": simulation_id,
        "stype": stat_type
    })

    print(f"Rows loaded: {len(df)}")

    if len(df) == 0:
        raise ValueError("Dataset is empty ❌")

    # =========================
    # 🔥 FEATURE ENGINEERING
    # =========================

    # 1. Strength (direction power)
    df["Strength"] = df["PureMean_Up"] - df["PureMean_Down"]

    # 2. Direction Bias
    df["DirectionBias"] = df["UpPercentage"] - df["DownPercentage"]

    # 3. Confidence (avoid division by zero)
    df["Confidence"] = df["Mean"] / (df["StdDev"] + 1e-6)

    # 4. Vote Strength
    df["VoteStrength"] = df["TotalVotes"] * df["Mean"].abs()

    # 5. Normalized Mean
    df["NormalizedMean"] = df["Mean"] / (df["StdDev"] + 1e-6)

    # =========================
    # CLEANUP (optional but smart)
    # =========================

    df = df.replace([float("inf"), float("-inf")], 0)
    df = df.fillna(0)

    # =========================
    # SAVE
    # =========================
    file_path = OUTPUT_DIR / f"dataset_sim_{simulation_id}_{stat_type}_v1.csv"

    df.to_csv(file_path, index=False)

    print(f"Dataset saved: {file_path}")

    return file_path