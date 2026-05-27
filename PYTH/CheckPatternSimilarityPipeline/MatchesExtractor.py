import json
import pandas as pd
from pathlib import Path


# =========================
# EXTRACT ALL JSON BLOCKS
# =========================
def extract_json_objects(content: str):
    """
    Extract multiple JSON objects from messy file
    """

    objs = []
    buffer = ""
    brace_count = 0

    for char in content:

        if char == "{":
            brace_count += 1

        if brace_count > 0:
            buffer += char

        if char == "}":
            brace_count -= 1

            if brace_count == 0 and buffer:
                try:
                    objs.append(json.loads(buffer))
                except Exception as e:
                    print(f"⚠️ Failed parsing one JSON block")
                buffer = ""

    return objs


# =========================
# LOAD JSON → DATAFRAME
# =========================
def load_matches(json_path: str) -> pd.DataFrame:

    print(f"\n=== Loading Matches from JSON ===")
    print(f"Path: {json_path}")

    path = Path(json_path)

    if not path.exists():
        raise FileNotFoundError(f"File not found: {json_path}")

    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    if not content.strip():
        raise ValueError("❌ File is empty")

    # =========================
    # EXTRACT ALL JSON OBJECTS
    # =========================
    json_objects = extract_json_objects(content)

    print(f"JSON blocks found: {len(json_objects)}")

    rows = []

    total_patterns = 0
    total_matches = 0
    bad_records = 0

    for data in json_objects:

        for current_tp, obj in data.items():

            total_patterns += 1

            matches = obj.get("ScoredTimePoints", [])

            if not matches:
                continue

            for m in matches:

                try:
                    rows.append({
                        "CurrentTP": int(current_tp),
                        "MatchTP": int(m["datetime"]),
                        "Score": float(m["score"])
                    })

                    total_matches += 1

                except Exception as e:
                    bad_records += 1
                    continue

    df = pd.DataFrame(rows)

    print(f"Patterns loaded: {total_patterns}")
    print(f"Matches loaded: {total_matches}")
    print(f"Bad records skipped: {bad_records}")
    print(f"Final rows: {len(df)}")

    if df.empty:
        raise ValueError("No matches loaded ❌")

    return df


# =========================
# OPTIONAL: SAVE TO CSV
# =========================
def save_matches(df: pd.DataFrame, output_path: str):

    print(f"\n=== Saving Matches ===")
    print(f"Path: {output_path}")

    df.to_csv(output_path, index=False)

    print("Saved ✔")


# =========================
# DEBUG / CLI
# =========================
if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser(description="Load Matches from JSON")
    parser.add_argument("json_path", type=str, help="Path to JSON file")
    parser.add_argument("--save", type=str, help="Optional CSV output path")

    args = parser.parse_args()

    df = load_matches(args.json_path)

    print("\n=== SAMPLE ===")
    print(df.head())

    if args.save:
        save_matches(df, args.save)