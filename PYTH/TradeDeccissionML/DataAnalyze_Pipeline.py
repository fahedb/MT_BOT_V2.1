from pathlib import Path

# === IMPORTS ===
from DirectionStats_Inject import inject_direction_stats
from FutureStatsBuild import compute_future_stats
from ExtractModelDataSet import extract_dataset, extract_combined_dataset
from LearningScript_TradeDec import run_model
from SaveModelResults import save_results


# =========================
# MAIN PIPELINE
# =========================
def run_pipeline(json_path: Path):

    print("\n==============================")
    print("🚀 START PIPELINE")
    print("==============================")

    # =========================
    # 1. Inject JSON → SQL
    # =========================
    sim_id = inject_direction_stats(json_path)

    if sim_id is None:
        raise ValueError("SimulationID not found ❌")

    print(f"\nSimulationID: {sim_id}")

    # =========================
    # 2. Compute FutureStats
    # =========================
    compute_future_stats(sim_id)

    # =========================
    # 3. Stat types כולל Combined
    # =========================
    stat_types = ["General", "First", "Highest", "Combined"]

    all_results = []

    for stat_type in stat_types:

        print("\n==============================")
        print(f"🔎 RUNNING STAT TYPE: {stat_type}")
        print("==============================")

        try:
            # =========================
            # Extract
            # =========================
            if stat_type == "Combined":
                csv_path = extract_combined_dataset(sim_id)
            else:
                csv_path = extract_dataset(sim_id, stat_type)

            # =========================
            # Run model
            # =========================
            results = run_model(csv_path, sim_id, stat_type)

            # =========================
            # Collect
            # =========================
            all_results.extend(results)

        except Exception as e:
            print(f"❌ Error in {stat_type}: {e}")
            continue

    # =========================
    # 4. Save Results → SQL
    # =========================
    if all_results:
        save_results(all_results)
    else:
        print("⚠️ No results to save")

    print("\n==============================")
    print("✅ PIPELINE FINISHED")
    print("==============================")


# =========================
# ENTRY POINT (חובה!)
# =========================
if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser(description="Full ML Pipeline")
    parser.add_argument("json_path", type=str, help="Path to DirectionStats JSON")

    args = parser.parse_args()

    # 🔥 Debug קטן (אפשר למחוק אחר כך)
    print(f"🔥 Running with JSON: {args.json_path}")

    run_pipeline(Path(args.json_path))