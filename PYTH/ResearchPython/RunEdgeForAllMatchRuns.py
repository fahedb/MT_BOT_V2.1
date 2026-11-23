import os
import json
import traceback

from sqlalchemy import create_engine, text

from EdgeFinder import run_edge_for_match_run

# =========================
# MATCH RUN SELECTION
# =========================

# Option 1:
# None = run all match runs with MatchesFile
MATCH_RUN_IDS = []

# Option 2:
# Use empty list [] if you want no explicit IDs
# MATCH_RUN_IDS = []

# Option 3:
# Optional range filter.
# If both are not None, script will run MatchRunID between them.
# Works together with MATCH_RUN_IDS only if MATCH_RUN_IDS is None or empty.
MATCH_RUN_ID_FROM = 86
MATCH_RUN_ID_TO = 94

# Safety limit for quick testing.
# None = no limit.
LIMIT_MATCH_RUNS = None

# If True, skip edge run if same MatchRunID + same EvaluationConfigJson already exists
SKIP_EXISTING = True
# =========================
# CONFIG
# =========================

# Connection string: override with the MTBOT_SQL_CONN environment variable.
# See docs/CONFIGURATION.md (refactor plan Phase 0, finding E12).
CONN_STR = os.environ.get(
    "MTBOT_SQL_CONN",
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes",
)

# None = run all match runs with MatchesFile
# Or set specific list:
# MATCH_RUN_IDS = [17, 29, 33, 44, 84, 89, 94]
MATCH_RUN_IDS = None

# Safety limit for quick testing
# None = no limit
LIMIT_MATCH_RUNS = None


EDGE_CONFIGS = [
    # =========================
    # FIRST HIT 150 PIPS - REGULAR AGREEMENT
    # =========================
    {
        "name": "first_hit_150_agree_07_valid_7_noise_03_w20",
        "prediction_method": "weighted_average",
        "weighting_method": "1_over_distance",
        "future_stats_function": "compute_future_stats_default",

        # Disable old return-based filters
        "min_direction_agreement": None,
        "min_weighted_direction_agreement": None,
        "min_consistency_score": None,
        "max_return_std": None,
        "min_matches_count": None,

        "future_direction_method": "first_hit_pips",
        "first_hit_pips": 150,
        "pip_size": 0.01,
        "first_hit_future_window": 20,
        "same_candle_policy": "noise",
        "ignore_noise_in_first_hit_prediction": True,
        "include_noise_in_accuracy": False,

        "min_first_hit_agreement": 0.7,
        "min_weighted_first_hit_agreement": None,
        "min_valid_first_hit_count": 7,
        "max_first_hit_noise_ratio": 0.3,
        "use_weighted_first_hit_prediction": False,
    },

    # =========================
    # FIRST HIT 150 PIPS - WEIGHTED AGREEMENT
    # =========================
    {
        "name": "first_hit_150_weighted_agree_07_valid_7_noise_03_w20",
        "prediction_method": "weighted_average",
        "weighting_method": "1_over_distance",
        "future_stats_function": "compute_future_stats_default",

        "min_direction_agreement": None,
        "min_weighted_direction_agreement": None,
        "min_consistency_score": None,
        "max_return_std": None,
        "min_matches_count": None,

        "future_direction_method": "first_hit_pips",
        "first_hit_pips": 150,
        "pip_size": 0.01,
        "first_hit_future_window": 20,
        "same_candle_policy": "noise",
        "ignore_noise_in_first_hit_prediction": True,
        "include_noise_in_accuracy": False,

        "min_first_hit_agreement": None,
        "min_weighted_first_hit_agreement": 0.7,
        "min_valid_first_hit_count": 7,
        "max_first_hit_noise_ratio": 0.3,
        "use_weighted_first_hit_prediction": True,
    },

    # =========================
    # FIRST HIT 200 PIPS - REGULAR AGREEMENT
    # =========================
    {
        "name": "first_hit_200_agree_07_valid_7_noise_03_w20",
        "prediction_method": "weighted_average",
        "weighting_method": "1_over_distance",
        "future_stats_function": "compute_future_stats_default",

        "min_direction_agreement": None,
        "min_weighted_direction_agreement": None,
        "min_consistency_score": None,
        "max_return_std": None,
        "min_matches_count": None,

        "future_direction_method": "first_hit_pips",
        "first_hit_pips": 200,
        "pip_size": 0.01,
        "first_hit_future_window": 20,
        "same_candle_policy": "noise",
        "ignore_noise_in_first_hit_prediction": True,
        "include_noise_in_accuracy": False,

        "min_first_hit_agreement": 0.7,
        "min_weighted_first_hit_agreement": None,
        "min_valid_first_hit_count": 7,
        "max_first_hit_noise_ratio": 0.3,
        "use_weighted_first_hit_prediction": False,
    },

    # =========================
    # FIRST HIT 200 PIPS - WEIGHTED AGREEMENT
    # =========================
    {
        "name": "first_hit_200_weighted_agree_07_valid_7_noise_03_w20",
        "prediction_method": "weighted_average",
        "weighting_method": "1_over_distance",
        "future_stats_function": "compute_future_stats_default",

        "min_direction_agreement": None,
        "min_weighted_direction_agreement": None,
        "min_consistency_score": None,
        "max_return_std": None,
        "min_matches_count": None,

        "future_direction_method": "first_hit_pips",
        "first_hit_pips": 200,
        "pip_size": 0.01,
        "first_hit_future_window": 20,
        "same_candle_policy": "noise",
        "ignore_noise_in_first_hit_prediction": True,
        "include_noise_in_accuracy": False,

        "min_first_hit_agreement": None,
        "min_weighted_first_hit_agreement": 0.7,
        "min_valid_first_hit_count": 7,
        "max_first_hit_noise_ratio": 0.3,
        "use_weighted_first_hit_prediction": True,
    },

    # =========================
    # FIRST HIT 250 PIPS - WEIGHTED AGREEMENT
    # =========================
    {
        "name": "first_hit_250_weighted_agree_07_valid_7_noise_03_w20",
        "prediction_method": "weighted_average",
        "weighting_method": "1_over_distance",
        "future_stats_function": "compute_future_stats_default",

        "min_direction_agreement": None,
        "min_weighted_direction_agreement": None,
        "min_consistency_score": None,
        "max_return_std": None,
        "min_matches_count": None,

        "future_direction_method": "first_hit_pips",
        "first_hit_pips": 250,
        "pip_size": 0.01,
        "first_hit_future_window": 20,
        "same_candle_policy": "noise",
        "ignore_noise_in_first_hit_prediction": True,
        "include_noise_in_accuracy": False,

        "min_first_hit_agreement": None,
        "min_weighted_first_hit_agreement": 0.7,
        "min_valid_first_hit_count": 7,
        "max_first_hit_noise_ratio": 0.3,
        "use_weighted_first_hit_prediction": True,
    },

    # =========================
    # FIRST HIT 300 PIPS - WEIGHTED AGREEMENT
    # =========================
    {
        "name": "first_hit_300_weighted_agree_07_valid_7_noise_03_w20",
        "prediction_method": "weighted_average",
        "weighting_method": "1_over_distance",
        "future_stats_function": "compute_future_stats_default",

        "min_direction_agreement": None,
        "min_weighted_direction_agreement": None,
        "min_consistency_score": None,
        "max_return_std": None,
        "min_matches_count": None,

        "future_direction_method": "first_hit_pips",
        "first_hit_pips": 300,
        "pip_size": 0.01,
        "first_hit_future_window": 20,
        "same_candle_policy": "noise",
        "ignore_noise_in_first_hit_prediction": True,
        "include_noise_in_accuracy": False,

        "min_first_hit_agreement": None,
        "min_weighted_first_hit_agreement": 0.7,
        "min_valid_first_hit_count": 7,
        "max_first_hit_noise_ratio": 0.3,
        "use_weighted_first_hit_prediction": True,
    },
]

# =========================
# DB HELPERS
# =========================

def create_engine_instance():
    return create_engine(CONN_STR)


def normalize_config_for_db(config):
    """
    Keep JSON stable so duplicate detection works.
    """
    return json.dumps(config, sort_keys=True)

def load_match_runs():
    engine = create_engine_instance()

    use_explicit_ids = MATCH_RUN_IDS is not None and len(MATCH_RUN_IDS) > 0
    use_range = (
        not use_explicit_ids and
        MATCH_RUN_ID_FROM is not None and
        MATCH_RUN_ID_TO is not None
    )

    if use_explicit_ids:
        placeholders = ", ".join([f":id{i}" for i in range(len(MATCH_RUN_IDS))])

        query = text(f"""
            SELECT
                MatchRunID,
                ExtractorsJson,
                ConfigJson,
                MatchesFile,
                CreatedAt
            FROM dbo.VectorMatchRuns
            WHERE MatchRunID IN ({placeholders})
              AND MatchesFile IS NOT NULL
              AND MatchesFile <> ''
            ORDER BY MatchRunID
        """)

        params = {
            f"id{i}": int(match_run_id)
            for i, match_run_id in enumerate(MATCH_RUN_IDS)
        }

    elif use_range:
        query = text("""
            SELECT
                MatchRunID,
                ExtractorsJson,
                ConfigJson,
                MatchesFile,
                CreatedAt
            FROM dbo.VectorMatchRuns
            WHERE MatchRunID BETWEEN :MatchRunIDFrom AND :MatchRunIDTo
              AND MatchesFile IS NOT NULL
              AND MatchesFile <> ''
            ORDER BY MatchRunID
        """)

        params = {
            "MatchRunIDFrom": int(MATCH_RUN_ID_FROM),
            "MatchRunIDTo": int(MATCH_RUN_ID_TO),
        }

    else:
        query = text("""
            SELECT
                MatchRunID,
                ExtractorsJson,
                ConfigJson,
                MatchesFile,
                CreatedAt
            FROM dbo.VectorMatchRuns
            WHERE MatchesFile IS NOT NULL
              AND MatchesFile <> ''
            ORDER BY MatchRunID
        """)

        params = {}

    with engine.begin() as conn:
        rows = conn.execute(query, params).fetchall()

    match_runs = []

    for row in rows:
        match_runs.append({
            "match_run_id": int(row[0]),
            "extractors_json": row[1],
            "config_json": row[2],
            "matches_file": row[3],
            "created_at": row[4],
        })

    if LIMIT_MATCH_RUNS is not None:
        match_runs = match_runs[:int(LIMIT_MATCH_RUNS)]

    return match_runs

def edge_run_exists(match_run_id, evaluation_config):
    engine = create_engine_instance()

    config_json = normalize_config_for_db(evaluation_config)

    query = text("""
        SELECT COUNT(*)
        FROM dbo.VectorEdgeRuns
        WHERE MatchRunID = :MatchRunID
          AND EvaluationConfigJson = :EvaluationConfigJson
    """)

    with engine.begin() as conn:
        count = conn.execute(query, {
            "MatchRunID": int(match_run_id),
            "EvaluationConfigJson": config_json
        }).scalar()

    return int(count) > 0


# =========================
# RUNNER
# =========================

def run_single_edge(match_run, edge_config, index, total):
    match_run_id = int(match_run["match_run_id"])
    matches_file = match_run["matches_file"]

    config_name = edge_config.get("name", "unnamed_config")

    print("\n" + "=" * 100)
    print(f"Running Edge {index}/{total}")
    print(f"MatchRunID: {match_run_id}")
    print(f"Config: {config_name}")
    print(f"MatchesFile: {matches_file}")
    print("=" * 100)

    # Remove nothing from config.
    # The name field is intentionally kept in EvaluationConfigJson
    # so it is easier to identify results in DB.

    if SKIP_EXISTING and edge_run_exists(match_run_id, edge_config):
        print("⏭️ Skipping existing EdgeRun")
        return {
            "success": True,
            "skipped": True,
            "match_run_id": match_run_id,
            "config_name": config_name,
            "edge_result": None,
            "error": None,
        }

    try:
        edge_result = run_edge_for_match_run(
            match_run_id=match_run_id,
            matches_file=matches_file,
            evaluation_config=edge_config
        )

        print("\n✅ Edge completed")
        print(f"EdgeRunID: {edge_result['edge_run_id']}")
        print(f"DatasetRows: {edge_result['dataset_rows']}")

        return {
            "success": True,
            "skipped": False,
            "match_run_id": match_run_id,
            "config_name": config_name,
            "edge_result": edge_result,
            "error": None,
        }

    except Exception:
        error_message = traceback.format_exc()

        print("\n❌ Edge failed")
        print(error_message)

        return {
            "success": False,
            "skipped": False,
            "match_run_id": match_run_id,
            "config_name": config_name,
            "edge_result": None,
            "error": error_message,
        }


def run():
    print("\n=== RUN EDGE FOR ALL MATCH RUNS ===")
    
    print("\n=== MATCH RUN SELECTION ===")
    print(f"MATCH_RUN_IDS: {MATCH_RUN_IDS}")
    print(f"MATCH_RUN_ID_FROM: {MATCH_RUN_ID_FROM}")
    print(f"MATCH_RUN_ID_TO: {MATCH_RUN_ID_TO}")
    print(f"LIMIT_MATCH_RUNS: {LIMIT_MATCH_RUNS}")
    print(f"SKIP_EXISTING: {SKIP_EXISTING}")
    
    match_runs = load_match_runs()

    print(f"MatchRuns found: {len(match_runs)}")
    print(f"Edge configs: {len(EDGE_CONFIGS)}")

    total_jobs = len(match_runs) * len(EDGE_CONFIGS)

    print(f"Total jobs: {total_jobs}")

    results = []

    job_index = 0

    for match_run in match_runs:
        print("\n" + "#" * 100)
        print(f"MatchRunID: {match_run['match_run_id']}")
        print(f"Extractors: {match_run['extractors_json']}")
        print("#" * 100)

        for edge_config in EDGE_CONFIGS:
            job_index += 1

            result = run_single_edge(
                match_run=match_run,
                edge_config=edge_config,
                index=job_index,
                total=total_jobs
            )

            results.append(result)

    success_count = sum(1 for r in results if r["success"] and not r["skipped"])
    skipped_count = sum(1 for r in results if r["skipped"])
    failed_count = sum(1 for r in results if not r["success"])

    print("\n" + "=" * 100)
    print("🏁 RUN EDGE FOR ALL MATCH RUNS FINISHED")
    print(f"Total jobs: {len(results)}")
    print(f"Success: {success_count}")
    print(f"Skipped: {skipped_count}")
    print(f"Failed: {failed_count}")
    print("=" * 100)

    if failed_count > 0:
        print("\nFailed jobs:")

        for r in results:
            if not r["success"]:
                print(
                    f"MatchRunID={r['match_run_id']} | "
                    f"Config={r['config_name']}"
                )

    return results


if __name__ == "__main__":
    run()