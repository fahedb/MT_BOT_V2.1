from pathlib import Path
import pandas as pd

# === IMPORTS שלך ===
from DataPrepare import build_dataset
from SplitData import split_dataset
from GenerateRules import generate_rules, Rule
from TrainEvaluator import evaluate_rules
from TestEvaluator import evaluate_on_test_for_db
from SaveRuleResults import save_rule_results


# =========================
# MAIN PIPELINE
# =========================
def run_pipeline(simulation_id: int, stat_type: str):

    print("\n==============================")
    print("🚀 START RULE PIPELINE")
    print("==============================")

    # =========================
    # 1. BUILD DATASET
    # =========================
    csv_path = build_dataset(simulation_id, stat_type)

    # =========================
    # 2. SPLIT
    # =========================
    train_path, test_path = split_dataset(csv_path)

    # =========================
    # 3. LOAD DATA
    # =========================
    df_train = pd.read_csv(train_path)
    df_test  = pd.read_csv(test_path)

    print(f"\nLoaded TRAIN: {len(df_train)} rows")
    print(f"Loaded TEST:  {len(df_test)} rows")

    # =========================
    # 4. SELECT FEATURE
    # =========================
    feature = "Strength"   # 🔥 אפשר לשנות

    # =========================
    # 5. GENERATE RULES
    # =========================
    rules = generate_rules(df_train, feature)

    # =========================
    # 6. TRAIN EVALUATION
    # =========================
    train_results = evaluate_rules(df_train, rules)

    # 🔥 הוספת Score
    train_results["Score"] = train_results["Edge"] * train_results["Trades"]

    print("\n=== TOP RULES (TRAIN) ===")
    print(train_results.head(10))

    # =========================
    # 7. SELECT TOP RULES
    # =========================
    top_n = 20

    top_rules_df = train_results.sort_values(by="Score", ascending=False).head(top_n)

    top_rules = [
        Rule(row["Feature"], row["Operator"], row["Threshold"])
        for _, row in top_rules_df.iterrows()
    ]

    # =========================
    # 8. TEST EVALUATION
    # =========================
    rows = evaluate_on_test_for_db(
        df_test,
        top_rules,
        top_rules_df,
        stat_type
    )

    print("\n=== TEST RESULTS (Preview) ===")
    for r in rows[:10]:
        print(r)

    # =========================
    # 9. SAVE TO SQL
    # =========================
    save_rule_results(rows)

    print("\n==============================")
    print("✅ PIPELINE FINISHED")
    print("==============================")


# =========================
# CLI ENTRY
# =========================
if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser(description="Rule Discovery Pipeline")

    parser.add_argument("simulation_id", type=int)
    parser.add_argument("stat_type", type=str)

    args = parser.parse_args()

    run_pipeline(args.simulation_id, args.stat_type)