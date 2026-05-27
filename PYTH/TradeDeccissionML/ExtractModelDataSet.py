import pandas as pd
from sqlalchemy import create_engine, text
from pathlib import Path

# =========================
# CONFIG
# =========================
CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

OUTPUT_DIR = Path(r"C:\\MT_Simulator_Exports\\ML_DATA")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)


# =========================
# FUNCTION
# =========================
def extract_dataset(simulation_id: int, stat_type: str):

    print(f"\n=== Extracting dataset | {stat_type} | SimulationID={simulation_id} ===")

    engine = create_engine(CONN_STR)

    query = text("""
    SELECT
        d.TimePoint,  -- 🔥 חשוב
        d.Mean,
        d.PureMean_Up,
        d.PureMean_Down,
        d.UpPercentage,
        d.DownPercentage,
        d.TotalVotes,
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
    ORDER BY d.TimePoint ASC
    """)

    df = pd.read_sql(
        query,
        engine,
        params={
            "sid": simulation_id,
            "stype": stat_type
        }
    )

    # 🔥 הגנה נוספת
    df = df.sort_values("TimePoint").reset_index(drop=True)

    print(f"Rows extracted: {len(df)}")

    if len(df) == 0:
        raise ValueError(f"Dataset is empty ❌ | StatType={stat_type}")

    # לא רוצים שהמודל ישתמש בזה
    df = df.drop(columns=["TimePoint"])

    file_path = OUTPUT_DIR / f"dataset_sim_{simulation_id}_{stat_type}.csv"

    df.to_csv(file_path, index=False)

    print(f"CSV saved: {file_path}")

    return file_path


# =========================
# COMBINED DATASET
# =========================
def extract_combined_dataset(simulation_id: int):

    print(f"\n=== Extracting COMBINED dataset | SimulationID={simulation_id} ===")

    engine = create_engine(CONN_STR)

    query = text("""
    SELECT
        g.TimePoint,  -- 🔥 חשוב

        -- ===== GENERAL =====
        g.Mean  AS G_Mean,
        g.PureMean_Up   AS G_Up,
        g.PureMean_Down AS G_Down,
        g.UpPercentage   AS G_UpPct,
        g.DownPercentage AS G_DownPct,
        g.TotalVotes     AS G_Votes,

        -- ===== FIRST =====
        f.Mean  AS F_Mean,
        f.PureMean_Up   AS F_Up,
        f.PureMean_Down AS F_Down,
        f.UpPercentage   AS F_UpPct,
        f.DownPercentage AS F_DownPct,
        f.TotalVotes     AS F_Votes,

        -- ===== HIGHEST =====
        h.Mean  AS H_Mean,
        h.PureMean_Up   AS H_Up,
        h.PureMean_Down AS H_Down,
        h.UpPercentage   AS H_UpPct,
        h.DownPercentage AS H_DownPct,
        h.TotalVotes     AS H_Votes,

        fs.IsWin

    FROM dbo.BackTest_DirectionStats g

    JOIN dbo.BackTest_DirectionStats f
        ON f.TimePoint = g.TimePoint
       AND f.SimulationID = g.SimulationID
       AND f.CoinsPair = g.CoinsPair
       AND f.VoteType = 'First'

    JOIN dbo.BackTest_DirectionStats h
        ON h.TimePoint = g.TimePoint
       AND h.SimulationID = g.SimulationID
       AND h.CoinsPair = g.CoinsPair
       AND h.VoteType = 'Highest'

    JOIN dbo.FutureStats fs
        ON fs.TimePoint = g.TimePoint
       AND fs.SimulationID = g.SimulationID
       AND fs.CoinsPair = g.CoinsPair

    WHERE g.VoteType = 'General'
      AND g.SimulationID = :sid
      AND fs.IsWin IS NOT NULL
      AND g.TotalVotes >= 2

    ORDER BY g.TimePoint ASC
    """)

    df = pd.read_sql(query, engine, params={"sid": simulation_id})

    # 🔥 הגנה נוספת
    df = df.sort_values("TimePoint").reset_index(drop=True)

    print(f"Rows extracted: {len(df)}")

    if len(df) == 0:
        raise ValueError("Combined dataset is empty ❌")

    # =========================
    # FEATURE ENGINEERING
    # =========================
    df["GF_UpDiff"] = df["G_UpPct"] - df["F_UpPct"]
    df["GH_UpDiff"] = df["G_UpPct"] - df["H_UpPct"]

    df["Momentum"] = df["F_Up"] - df["G_Up"]

    df["Strength_G"] = df["G_Up"] - df["G_Down"]
    df["Strength_F"] = df["F_Up"] - df["F_Down"]
    df["Strength_H"] = df["H_Up"] - df["H_Down"]

    # לא רוצים leakage
    df = df.drop(columns=["TimePoint"])

    file_path = OUTPUT_DIR / f"dataset_sim_{simulation_id}_COMBINED.csv"

    df.to_csv(file_path, index=False)

    print(f"CSV saved: {file_path}")

    return file_path