# ==========================
# CONFIGURATION (EDIT HERE)
# ==========================

JSON_FILE_PATH = r"C:\\MT_Simulator_Exports\\TEST\\ClosedDeals.json"

SQL_CONNECTION_STRING = (
    "DRIVER={SQL Server};SERVER=localhost\\sqlexpress;DATABASE=Prices;Integrated Security=SSPI;"

)

COMMIT_EVERY = 1000   # 0 = commit once at end


import json
from pathlib import Path
from typing import Any, Dict, List, Tuple

import pyodbc


MERGE_SQL = r"""
MERGE dbo.BackTest_Deals AS tgt
USING (VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)) AS src
(
    SimulationID,
    symbol,
    ticket,
    openTime,
    closeTime,
    lotSize,
    openPrice,
    closePrice,
    takeProfitPrice,
    stopLossPrice,
    dealType,
    profit,
    profitInPips,
    closeReason,
    numOfBarsHeld
)
ON  tgt.SimulationID = src.SimulationID
AND tgt.ticket       = src.ticket
WHEN MATCHED THEN
    UPDATE SET
        symbol          = src.symbol,
        openTime        = src.openTime,
        closeTime       = src.closeTime,
        lotSize         = src.lotSize,
        openPrice       = src.openPrice,
        closePrice      = src.closePrice,
        takeProfitPrice = src.takeProfitPrice,
        stopLossPrice   = src.stopLossPrice,
        dealType        = src.dealType,
        profit          = src.profit,
        profitInPips    = src.profitInPips,
        closeReason     = src.closeReason,
        numOfBarsHeld   = src.numOfBarsHeld
WHEN NOT MATCHED THEN
    INSERT
    (
        SimulationID,
        symbol,
        ticket,
        openTime,
        closeTime,
        lotSize,
        openPrice,
        closePrice,
        takeProfitPrice,
        stopLossPrice,
        dealType,
        profit,
        profitInPips,
        closeReason,
        numOfBarsHeld
    )
    VALUES
    (
        src.SimulationID,
        src.symbol,
        src.ticket,
        src.openTime,
        src.closeTime,
        src.lotSize,
        src.openPrice,
        src.closePrice,
        src.takeProfitPrice,
        src.stopLossPrice,
        src.dealType,
        src.profit,
        src.profitInPips,
        src.closeReason,
        src.numOfBarsHeld
    );
"""


def load_json_records(path: Path) -> List[Dict[str, Any]]:
    """
    תומך ב:
      1) JSON יחיד: {...}
      2) מערך JSON: [{...}, {...}]
      3) JSON Lines: כל שורה היא {...}
    """
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

    records: List[Dict[str, Any]] = []
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


def to_str(x: Any, field: str) -> str:
    s = str(x).strip()
    if not s:
        raise ValueError(f"Field '{field}' is empty. Value={x!r}")
    return s


def json_to_row(j: Dict[str, Any]) -> Tuple[Any, ...]:
    return (
        to_int(require(j, "SimulationID"), "SimulationID"),
        to_str(require(j, "symbol"), "symbol"),
        to_int(require(j, "ticket"), "ticket"),
        to_int(require(j, "openTime"), "openTime"),
        to_int(require(j, "closeTime"), "closeTime"),
        to_float(require(j, "lotSize"), "lotSize"),
        to_float(require(j, "openPrice"), "openPrice"),
        to_float(require(j, "closePrice"), "closePrice"),
        to_float(require(j, "TakeProfitPrice"), "TakeProfitPrice"),
        to_float(require(j, "StopLossPrice"), "StopLossPrice"),
        to_str(require(j, "type"), "type"),  # JSON: type -> DB: dealType
        to_float(require(j, "profit"), "profit"),
        to_int(require(j, "profitInPips"), "profitInPips"),
        to_str(require(j, "closeReason"), "closeReason"),
        to_int(require(j, "numOfBarsHeld"), "numOfBarsHeld"),
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
        print(f"Inserted/updated {total} deals rows")

    except Exception:
        cnxn.rollback()
        raise
    finally:
        cnxn.close()


if __name__ == "__main__":
    main()
