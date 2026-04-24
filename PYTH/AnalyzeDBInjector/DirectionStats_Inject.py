import json
from pathlib import Path
from typing import Any, Dict, List, Tuple

import pyodbc

# ==========================
# CONFIGURATION (EDIT HERE)
# ==========================

JSON_FILE_PATH = r"C:\\MT_Simulator_Exports\\TEST\\DirectionStats.json"

SQL_CONNECTION_STRING = (
    "DRIVER={SQL Server};SERVER=localhost\\sqlexpress;DATABASE=Prices;Integrated Security=SSPI;"
)

TRADE_RESULT = 0          # NOT NULL in DB
COMMIT_EVERY = 1000       # 0 = commit once at end

# ==========================

VOTE_BLOCKS = [ 
    ("First_VotedPercentages", "First"),
    ("General_VotedPercentages", "General"),
    ("Highest_VotedPercentages", "Highest"),
]


MERGE_SQL = r"""
MERGE dbo.BackTest_DirectionStats AS tgt
USING (VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)) AS src
(
    SimulationID, TimePoint, CoinsPair, VoteType, TotalVotes,
    UpPercentage, DownPercentage, BlurPercentage,
    Mean, PureMean_Up, PureMean_Down, StdDev, MeanIndexes, TradeResult
)
ON  tgt.SimulationID = src.SimulationID
AND tgt.VoteType     = src.VoteType
AND tgt.TimePoint    = src.TimePoint
WHEN MATCHED THEN
    UPDATE SET
        CoinsPair      = src.CoinsPair,
        TotalVotes     = src.TotalVotes,
        UpPercentage   = src.UpPercentage,
        DownPercentage = src.DownPercentage,
        BlurPercentage = src.BlurPercentage,
        Mean           = src.Mean,
        PureMean_Up    = src.PureMean_Up,
        PureMean_Down  = src.PureMean_Down,
        StdDev         = src.StdDev,
        MeanIndexes    = src.MeanIndexes,
        TradeResult    = src.TradeResult
WHEN NOT MATCHED THEN
    INSERT
    (
        SimulationID, TimePoint, CoinsPair, VoteType, TotalVotes,
        UpPercentage, DownPercentage, BlurPercentage,
        Mean, PureMean_Up, PureMean_Down, StdDev, MeanIndexes, TradeResult
    )
    VALUES
    (
        src.SimulationID, src.TimePoint, src.CoinsPair, src.VoteType, src.TotalVotes,
        src.UpPercentage, src.DownPercentage, src.BlurPercentage,
        src.Mean, src.PureMean_Up, src.PureMean_Down, src.StdDev, src.MeanIndexes, src.TradeResult
    );
"""


def load_json_records(path: Path) -> List[Dict[str, Any]]:
    raw = path.read_text(encoding="utf-8").strip()
    if not raw:
        return []

    try:
        obj = json.loads(raw)
        if isinstance(obj, dict):
            return [obj]
        if isinstance(obj, list):
            return obj
    except json.JSONDecodeError:
        pass

    records = []
    for line in raw.splitlines():
        line = line.strip()
        if line:
            records.append(json.loads(line))
    return records


def require(j: Dict[str, Any], key: str) -> Any:
    if key not in j or j[key] is None:
        raise ValueError(f"Missing required field '{key}'")
    return j[key]


def json_to_rows(j: Dict[str, Any]) -> List[Tuple[Any, ...]]:
    simulation_id = int(require(j, "SimulationID"))
    time_point = int(require(j, "TimePoint"))
    pair = str(require(j, "Pair")).strip()

    if not pair:
        raise ValueError("Pair is empty")

    rows = []

    for block_key, vote_type in VOTE_BLOCKS:
        b = require(j, block_key)

        rows.append(
            (
                simulation_id,
                time_point,
                pair,
                vote_type,
                int(b["TotalVotes"]),
                float(b["UP_Percent"]),
                float(b["DOWN_Percent"]),
                float(b["BLUR_Percent"]),
                float(b["Mean"]),
                float(b["PureMean_UP"]),
                float(b["PureMean_DOWN"]),
                float(b["StdDev"]),
                float(b["MeanIndexes"]),
                TRADE_RESULT,
            )
        )

    return rows


def main() -> None:
    records = load_json_records(Path(JSON_FILE_PATH))
    if not records:
        print("No JSON records found.")
        return

    cnxn = pyodbc.connect(SQL_CONNECTION_STRING, autocommit=False)
    try:
        cur = cnxn.cursor()
        total_rows = 0

        for i, j in enumerate(records, start=1):
            rows = json_to_rows(j)
            for row in rows:
                cur.execute(MERGE_SQL, row)
            total_rows += len(rows)

            if COMMIT_EVERY > 0 and i % COMMIT_EVERY == 0:
                cnxn.commit()

        cnxn.commit()
        print(f"Inserted/updated {total_rows} rows from {len(records)} JSON records")

    except Exception:
        cnxn.rollback()
        raise
    finally:
        cnxn.close()


if __name__ == "__main__":
    main()
