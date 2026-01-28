# ==========================
# CONFIGURATION (EDIT HERE)
# ==========================

JSON_FILE_PATH = r"C:\\MT_Simulator_Exports\\TEST\\AccountState.json"

SQL_CONNECTION_STRING = (
    "DRIVER={SQL Server};SERVER=localhost;DATABASE=Prices;Integrated Security=SSPI;"
)

COMMIT_EVERY = 1000   # 0 = commit once at end
# ==========================

import json
from pathlib import Path
from typing import Any, Dict, List, Tuple

import pyodbc


MERGE_SQL = r"""
MERGE dbo.BackTest_AccountStateEvents AS tgt
USING (VALUES (?, ?, ?, ?, ?, ?, ?, ?)) AS src
(
    SimulationID,
    CurrTimePoint,
    AccountBalance,
    EquityBalance,
    UsedMargin,
    FreeMargin,
    FloatingPnL,
    CurrProfit
)
ON  tgt.SimulationID  = src.SimulationID
AND tgt.CurrTimePoint = src.CurrTimePoint
WHEN MATCHED THEN
    UPDATE SET
        AccountBalance = src.AccountBalance,
        EquityBalance  = src.EquityBalance,
        UsedMargin     = src.UsedMargin,
        FreeMargin     = src.FreeMargin,
        FloatingPnL    = src.FloatingPnL,
        CurrProfit     = src.CurrProfit
WHEN NOT MATCHED THEN
    INSERT
    (
        SimulationID,
        CurrTimePoint,
        AccountBalance,
        EquityBalance,
        UsedMargin,
        FreeMargin,
        FloatingPnL,
        CurrProfit
    )
    VALUES
    (
        src.SimulationID,
        src.CurrTimePoint,
        src.AccountBalance,
        src.EquityBalance,
        src.UsedMargin,
        src.FreeMargin,
        src.FloatingPnL,
        src.CurrProfit
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
            return [x for x in obj if isinstance(x, dict)]
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
        raise ValueError(f"Missing required field '{key}' in JSON: {j}")
    return j[key]


def to_int(x: Any, field: str) -> int:
    try:
        return int(x)
    except Exception as e:
        raise ValueError(f"Failed to convert '{field}' to int. Value={x!r}") from e


def to_float(x: Any, field: str) -> float:
    try:
        return float(x)
    except Exception as e:
        raise ValueError(f"Failed to convert '{field}' to float. Value={x!r}") from e


def json_to_row(j: Dict[str, Any]) -> Tuple[Any, ...]:
    return (
        to_int(require(j, "SimulationID"), "SimulationID"),
        to_int(require(j, "CurrTimePoint"), "CurrTimePoint"),
        to_float(require(j, "AccountBalance"), "AccountBalance"),
        to_float(require(j, "EquityBalance"), "EquityBalance"),
        to_float(require(j, "UsedMargin"), "UsedMargin"),
        to_float(require(j, "FreeMargin"), "FreeMargin"),
        to_float(require(j, "FloatingPnL"), "FloatingPnL"),
        to_float(require(j, "CurrProfit"), "CurrProfit"),
    )


def main() -> None:
    records = load_json_records(Path(JSON_FILE_PATH))
    if not records:
        print("No JSON records found.")
        return

    cnxn = pyodbc.connect(SQL_CONNECTION_STRING, autocommit=False)
    try:
        cur = cnxn.cursor()
        total = 0

        for i, j in enumerate(records, start=1):
            row = json_to_row(j)
            cur.execute(MERGE_SQL, row)
            total += 1

            if COMMIT_EVERY > 0 and i % COMMIT_EVERY == 0:
                cnxn.commit()

        cnxn.commit()
        print(f"Inserted/updated {total} account state rows")

    except Exception:
        cnxn.rollback()
        raise
    finally:
        cnxn.close()


if __name__ == "__main__":
    main()
