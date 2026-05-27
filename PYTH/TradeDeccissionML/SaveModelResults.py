from sqlalchemy import create_engine, text
from datetime import datetime

CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

def save_results(results: list):

    print(f"\n=== Saving ML results ===")

    engine = create_engine(CONN_STR)

    now = datetime.utcnow()

    rows = []

    for r in results:

        winrate = r["WinRate"]
        total = r["TotalTrades"]

        edge = winrate - 0.5

        # 🔥 מדד הרבה יותר חשוב
        score = edge * total

        rows.append({
            "SimulationID": r["SimulationID"],
            "ModelType": r["ModelType"],
            "StatType": r["StatType"],
            "MaxDepth": r["MaxDepth"],
            "Threshold": r["Threshold"],
            "TotalTrades": total,
            "WinRate": winrate,
            "Accuracy": r["Accuracy"],
            "Edge": edge,
            "Score": score,               # 🔥 חדש
            "CreatedAt": now              # 🔥 חדש
        })

    # =========================
    # DELETE DUPLICATES (same run)
    # =========================
    delete_sql = text("""
        DELETE FROM dbo.ML_ExperimentResults
        WHERE SimulationID = :sid
    """)

    # =========================
    # INSERT
    # =========================
    insert_sql = text("""
        INSERT INTO dbo.ML_ExperimentResults
        (SimulationID, ModelType, StatType, MaxDepth, Threshold,
         TotalTrades, WinRate, Accuracy, Edge, Score, CreatedAt)
        VALUES (:SimulationID, :ModelType, :StatType, :MaxDepth, :Threshold,
                :TotalTrades, :WinRate, :Accuracy, :Edge, :Score, :CreatedAt)
    """)

    with engine.begin() as conn:

        # 🔥 מנקה תוצאות קודמות של אותו סימולציה
        conn.execute(delete_sql, {"sid": results[0]["SimulationID"]})

        # 🔥 מכניס חדש
        conn.execute(insert_sql, rows)

    print(f"Inserted {len(rows)} rows ✔")