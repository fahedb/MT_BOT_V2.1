import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Tuple

import pyodbc

# ==========================
# CONFIGURATION
# ==========================

SQL_CONNECTION_STRING = (
    "DRIVER={SQL Server};SERVER=localhost\\sqlexpress;DATABASE=Prices;Integrated Security=SSPI;"
)

TRADE_RESULT = 0
COMMIT_EVERY = 1000

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

# ==========================
# JSON HELPERS
# ==========================

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


def require_block(j: Dict[str, Any], key: str) -> Dict[str, Any]:
    if key not in j or j[key] is None:
        raise ValueError(f"Missing block '{key}'")
    return j[key]


def safe_float(v, field_name):
    if v is None:
        raise ValueError(f"{field_name} is None")
    return float(v)


def json_to_rows(j: Dict[str, Any]) -> List[Tuple[Any, ...]]:

    simulation_id = int(require(j, "SimulationID"))
    time_point = int(require(j, "TimePoint"))
    pair = str(require(j, "Pair")).strip()

    if not pair:
        raise ValueError("Pair is empty")

    rows = []

    for block_key, vote_type in VOTE_BLOCKS:

        b = require_block(j, block_key)

        try:
            row = (
                simulation_id,
                time_point,
                pair,
                vote_type,

                int(b["TotalVotes"]),
                safe_float(b["UP_Percent"], "UP_Percent"),
                safe_float(b["DOWN_Percent"], "DOWN_Percent"),
                safe_float(b["BLUR_Percent"], "BLUR_Percent"),

                safe_float(b["Mean"], "Mean"),
                safe_float(b["PureMean_UP"], "PureMean_UP"),
                safe_float(b["PureMean_DOWN"], "PureMean_DOWN"),

                safe_float(b["StdDev"], "StdDev"),
                safe_float(b["MeanIndexes"], "MeanIndexes"),

                TRADE_RESULT,
            )

        except KeyError as e:
            raise KeyError(f"Missing key inside {block_key}: {e}")

        rows.append(row)

    return rows


# ==========================
# CORE FUNCTION (PIPELINE)
# ==========================

def inject_direction_stats(json_path: Path) -> int:

    if not json_path.exists():
        raise FileNotFoundError(f"JSON file not found: {json_path}")

    print(f"Loading JSON from: {json_path}")

    records = load_json_records(json_path)
    if not records:
        print("No JSON records found.")
        return None

    simulation_id = int(records[0]["SimulationID"])
    print(f"Running SimulationID={simulation_id}")

    cnxn = pyodbc.connect(SQL_CONNECTION_STRING, autocommit=False)

    bad_records = []

    try:
        cur = cnxn.cursor()
        total_rows = 0

        for i, j in enumerate(records, start=1):

            try:
                rows = json_to_rows(j)

                for row in rows:
                    cur.execute(MERGE_SQL, row)

                total_rows += len(rows)

            except Exception as e:
                print("\n❌ BAD RECORD FOUND")
                print(f"Index: {i}")
                print(f"TimePoint: {j.get('TimePoint')}")
                print(f"Error: {e}")
                print(f"Keys: {list(j.keys())}")

                bad_records.append((i, j.get("TimePoint"), str(e)))
                continue

            if COMMIT_EVERY > 0 and i % COMMIT_EVERY == 0:
                cnxn.commit()

        cnxn.commit()

        print(f"\nInserted/updated {total_rows} rows from {len(records)} JSON records")
        print(f"⚠️ Bad records skipped: {len(bad_records)}")

    except Exception:
        cnxn.rollback()
        raise

    finally:
        cnxn.close()

    return simulation_id


# ==========================
# CLI ENTRY POINT
# ==========================

def main():
    parser = argparse.ArgumentParser(description="Inject DirectionStats JSON to SQL")
    parser.add_argument("json_path", type=str, help="Path to JSON file")

    args = parser.parse_args()

    sim_id = inject_direction_stats(Path(args.json_path))

    print(f"SimulationID loaded: {sim_id}")


if __name__ == "__main__":
    main()