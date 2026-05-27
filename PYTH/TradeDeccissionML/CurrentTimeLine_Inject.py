import json
from decimal import Decimal, InvalidOperation, ROUND_HALF_UP

import pyodbc


# =========================
# Configuration
# =========================
TIMEFRAMES = ["M1", "M15", "H1","H4"]
LENS = [10, 20, 30]

JSON_FILE_PATH = r"C:\\MTDB\\BackTestCurrData\\XAUUSD_M1_DATA_SHORT.json"
COINS_PAIR = "XAUUSD"

BATCH_SIZE = 1000

# 🔥 FLAG לשליטה על אינדיקטורים
ENABLE_INDICATORS = True


SQL_SCALE = Decimal("0.00000001")


def to_decimal(value):
    """Convert a value to Decimal safely and fit SQL DECIMAL(18,8)."""
    if value is None:
        return None

    try:
        return Decimal(str(value)).quantize(SQL_SCALE, rounding=ROUND_HALF_UP)
    except (InvalidOperation, ValueError, TypeError):
        return None


def parse_ind_len(ind_len_key):
    """Convert 'L10' -> 10."""
    if not isinstance(ind_len_key, str):
        return None

    if not ind_len_key.startswith("L"):
        return None

    try:
        return int(ind_len_key[1:])
    except ValueError:
        return None


def build_connection():
    conn_str = (
        "DRIVER={SQL Server};"
        "SERVER=localhost\\sqlexpress;"
        "DATABASE=Prices;"
        "Trusted_Connection=yes;"
    )
    return pyodbc.connect(conn_str)


def insert_batches(cursor, candle_rows, indicator_rows):
    """Insert batch rows into SQL Server."""

    if candle_rows:
        cursor.executemany(
            """
            INSERT INTO dbo.CurrentTimeLine
            (
                TimePoint,
                CoinsPair,
                OpenPrice,
                ClosePrice,
                HighPrice,
                LowPrice
            )
            VALUES (?, ?, ?, ?, ?, ?)
            """,
            candle_rows,
        )

    if ENABLE_INDICATORS and indicator_rows:
        cursor.executemany(
            """
            INSERT INTO dbo.CurrentIndicatorTimeLine
            (
                TimePoint,
                CoinsPair,
                IndicatorName,
                TimeFrame,
                IndLen,
                IndicatorValue,
                IndicatorSlope
            )
            VALUES (?, ?, ?, ?, ?, ?, ?)
            """,
            indicator_rows,
        )


def process_file():
    conn = build_connection()
    conn.autocommit = False
    cursor = conn.cursor()
    cursor.fast_executemany = True

    candle_rows = []
    indicator_rows = []

    total_lines = 0
    total_candles = 0
    total_indicator_rows = 0

    with open(JSON_FILE_PATH, "r", encoding="utf-8") as file:
        for line_number, line in enumerate(file, start=1):
            line = line.strip()
            if not line:
                continue

            total_lines += 1

            try:
                json_obj = json.loads(line)
            except json.JSONDecodeError as exc:
                print(f"[Line {line_number}] Invalid JSON: {exc}")
                continue

            for time_point_str, candle_data in json_obj.items():
                try:
                    time_point = int(time_point_str)
                except ValueError:
                    print(f"[Line {line_number}] Invalid TimePoint: {time_point_str}")
                    continue

                open_price = to_decimal(candle_data.get("OpenPrice"))
                close_price = to_decimal(candle_data.get("ClosePrice"))
                high_price = to_decimal(candle_data.get("HighPrice"))
                low_price = to_decimal(candle_data.get("LowPrice"))

                candle_rows.append(
                    (
                        time_point,
                        COINS_PAIR,
                        open_price,
                        close_price,
                        high_price,
                        low_price,
                    )
                )
                total_candles += 1

                # 🔥 אינדיקטורים רק אם מופעל
                if ENABLE_INDICATORS:
                    indicators = candle_data.get("indicators", {})
                    if isinstance(indicators, dict):

                        for indicator_name, indicator_lengths in indicators.items():
                            if indicator_name == "ZLSMA":
                                continue;

                            if not isinstance(indicator_lengths, dict):
                                continue

                            for ind_len_key, ind_len_data in indicator_lengths.items():
                                if not isinstance(ind_len_data, dict):
                                    continue

                                ind_len = parse_ind_len(ind_len_key)
                                if ind_len is None:
                                    continue
                                
                                if ind_len not in LENS:
                                    continue

                                values_block = ind_len_data.get("Values", {})
                                slopes_block = ind_len_data.get("Slopes", {})

                                if not isinstance(values_block, dict):
                                    values_block = {}

                                if not isinstance(slopes_block, dict):
                                    slopes_block = {}

                                all_timeframes = set(values_block.keys()) | set(slopes_block.keys())

                                for timeframe in all_timeframes:
                                    if timeframe not in TIMEFRAMES:
                                        continue;

                                    indicator_value = to_decimal(values_block.get(timeframe))
                                    indicator_slope = to_decimal(slopes_block.get(timeframe))

                                    indicator_rows.append(
                                        (
                                            time_point,
                                            COINS_PAIR,
                                            indicator_name,
                                            timeframe,
                                            ind_len,
                                            indicator_value,
                                            indicator_slope,
                                        )
                                    )
                                    total_indicator_rows += 1

                # 🔥 Batch insert
                if len(candle_rows) >= BATCH_SIZE:
                    try:
                        insert_batches(cursor, candle_rows, indicator_rows)
                        conn.commit()

                        print(
                            f"Committed batch: "
                            f"candles={len(candle_rows)}, "
                            f"indicator_rows={len(indicator_rows) if ENABLE_INDICATORS else 0}"
                        )
                    except Exception as exc:
                        conn.rollback()
                        print(f"Batch failed at line {line_number}: {exc}")
                        raise

                    candle_rows.clear()
                    indicator_rows.clear()

    # 🔥 Final batch
    if candle_rows or (ENABLE_INDICATORS and indicator_rows):
        try:
            insert_batches(cursor, candle_rows, indicator_rows)
            conn.commit()

            print(
                f"Committed final batch: "
                f"candles={len(candle_rows)}, "
                f"indicator_rows={len(indicator_rows) if ENABLE_INDICATORS else 0}"
            )
        except Exception as exc:
            conn.rollback()
            print(f"Final batch failed: {exc}")
            raise

    cursor.close()
    conn.close()

    print("Done.")
    print(f"Total JSON lines processed: {total_lines}")
    print(f"Total candles inserted: {total_candles}")
    print(f"Total indicator rows inserted: {total_indicator_rows if ENABLE_INDICATORS else 0}")


if __name__ == "__main__":
    process_file()