import pandas as pd
from pathlib import Path


# =========================
# FUNCTION
# =========================
def split_dataset(csv_path: str, train_ratio: float = 0.7):

    print(f"\n=== Splitting dataset ===")
    print(f"Input: {csv_path}")

    df = pd.read_csv(csv_path)

    if "TimePoint" not in df.columns:
        raise ValueError("TimePoint column is missing ❌")

    # =========================
    # Ensure sorted by time
    # =========================
    df = df.sort_values("TimePoint").reset_index(drop=True)

    total_rows = len(df)
    split_index = int(total_rows * train_ratio)

    train_df = df.iloc[:split_index].copy()
    test_df  = df.iloc[split_index:].copy()

    print(f"Total rows: {total_rows}")
    print(f"Train rows: {len(train_df)}")
    print(f"Test rows:  {len(test_df)}")

    # =========================
    # Save files
    # =========================
    base_path = Path(csv_path)

    train_path = base_path.with_name(base_path.stem + "_TRAIN.csv")
    test_path  = base_path.with_name(base_path.stem + "_TEST.csv")

    train_df.to_csv(train_path, index=False)
    test_df.to_csv(test_path, index=False)

    print(f"Saved TRAIN: {train_path}")
    print(f"Saved TEST:  {test_path}")

    return train_path, test_path