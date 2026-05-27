from sqlalchemy import create_engine, text

CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)


def save_rule_results(rows):

    print("\n=== Saving Rule Results ===")

    engine = create_engine(CONN_STR)

    insert_sql = text("""
        INSERT INTO dbo.ML_RuleResults
        (
            RuleDescription,
            StatType,

            TrainTrades, TrainWinRate, TrainEdge,
            TestTrades, TestWinRate, TestEdge,

            Delta, Stability, Score
        )
        VALUES
        (
            :RuleDescription,
            :StatType,

            :TrainTrades, :TrainWinRate, :TrainEdge,
            :TestTrades, :TestWinRate, :TestEdge,

            :Delta, :Stability, :Score
        )
    """)

    with engine.begin() as conn:
        conn.execute(insert_sql, rows)

    print(f"Inserted {len(rows)} rows ✔")