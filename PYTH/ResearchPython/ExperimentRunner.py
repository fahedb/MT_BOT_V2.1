from concurrent.futures import ProcessPoolExecutor, as_completed
import traceback

from BuildMatchesCache import build_matches_for_config
from EdgeFinder import run_edge_for_match_run

from ExperimentConfigs.ExtractorCombinations import (
    build_extractor_combination_configs,
    ALL_EXTRACTORS
)
from VectorBuilders.VolatilityVectorBuilder import extract as volatility_extract
from VectorBuilders.CandleStructureVectorBuilder import extract as candle_structure_extract
from VectorBuilders.H1TrendVectorBuilder import extract as h1_trend_extract

from VectorBuilders.RangeLocationVectorBuilder import extract as range_location_extract
from VectorBuilders.RsiDirectionVectorBuilder import extract as rsi_direction_extract
from VectorBuilders.RsiLevelVectorBuilder import extract as rsi_level_extract
from VectorBuilders.RsiExtremeVectorBuilder import extract as rsi_extreme_extract
from VectorBuilders.EmaDirectionVectorBuilder import extract as ema_direction_extract
from VectorBuilders.EmaDistanceVectorBuilder import extract as ema_distance_extract
# =========================
# CONFIG
# =========================

RUN_EDGE_AFTER_MATCHES = True

TEST_LIMIT = None

# חשוב להתחיל עם 2 בגלל RAM
MAX_WORKERS = 5


# =========================
# BUILD EXPERIMENT CONFIGS
# =========================
def build_experiment_configs():

    configs = []

    # =========================
    # Strongest base so far
    # =========================
    base_extractors_config = [
        ("VolatilityVectorBuilder", volatility_extract),
        ("CandleStructureVectorBuilder", candle_structure_extract),
        ("H1TrendVectorBuilder", h1_trend_extract),
    ]

    # =========================
    # New / experimental extractors
    # =========================
    candidate_extractors_config = [
        ("RangeLocationVectorBuilder", range_location_extract),

        ("RsiDirectionVectorBuilder", rsi_direction_extract),
        ("RsiLevelVectorBuilder", rsi_level_extract),
        ("RsiExtremeVectorBuilder", rsi_extreme_extract),

        ("EmaDirectionVectorBuilder", ema_direction_extract),
        ("EmaDistanceVectorBuilder", ema_distance_extract),
    ]

    configs += build_extractor_combination_configs(
        base_extractors_config=base_extractors_config,
        candidate_extractors_config=candidate_extractors_config,

        # This means:
        # Base + 1 extra extractor
        # Base + 2 extra extractors
        min_size=3,
        max_size=None,

        window_size=30,
        top_k=10,
        distance_threshold=0.8,
        normalization_method="feature_axis_0",

        include_base_only=False,
        limit=TEST_LIMIT
    )

    return configs

# =========================
# RUN SINGLE EXPERIMENT
# =========================
def run_single_experiment(config):

    try:
        print("\n" + "=" * 80)
        print("🚀 Running experiment")
        print(f"Type: {config.get('experiment_type')}")
        print(f"Extractors: {config.get('extractor_names')}")
        print(f"WindowSize: {config.get('window_size')}")
        print(f"TopK: {config.get('top_k')}")
        print(f"DistanceThreshold: {config.get('distance_threshold')}")
        print("=" * 80)

        match_result = build_matches_for_config(
            extractors_config=config["extractors_config"],
            window_size=config["window_size"],
            top_k=config["top_k"],
            distance_threshold=config["distance_threshold"],
            normalization_method=config["normalization_method"],
            use_status_tracking=False
        )

        edge_result = None

        if RUN_EDGE_AFTER_MATCHES:
            edge_result = run_edge_for_match_run(
                match_run_id=match_result["match_run_id"],
                matches_file=match_result["matches_file"]
            )

        return {
            "success": True,
            "config": {
                "experiment_type": config.get("experiment_type"),
                "extractor_names": config.get("extractor_names"),
                "window_size": config.get("window_size"),
                "top_k": config.get("top_k"),
                "distance_threshold": config.get("distance_threshold"),
                "normalization_method": config.get("normalization_method"),
            },
            "match_result": match_result,
            "edge_result": edge_result,
            "error": None
        }

    except Exception:
        error_message = traceback.format_exc()

        return {
            "success": False,
            "config": {
                "experiment_type": config.get("experiment_type"),
                "extractor_names": config.get("extractor_names"),
                "window_size": config.get("window_size"),
                "top_k": config.get("top_k"),
                "distance_threshold": config.get("distance_threshold"),
                "normalization_method": config.get("normalization_method"),
            },
            "match_result": None,
            "edge_result": None,
            "error": error_message
        }


# =========================
# MAIN PARALLEL RUNNER
# =========================
def run():

    print("\n=== PARALLEL EXPERIMENT RUNNER ===")

    configs = build_experiment_configs()

    print(f"Total configs: {len(configs)}")
    print(f"MAX_WORKERS: {MAX_WORKERS}")

    results = []

    with ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:

        future_to_config = {
            executor.submit(run_single_experiment, config): config
            for config in configs
        }

        completed = 0
        total = len(configs)

        for future in as_completed(future_to_config):

            completed += 1
            config = future_to_config[future]

            try:
                result = future.result()
                results.append(result)

                print("\n" + "-" * 80)
                print(f"Completed {completed}/{total}")
                print(f"Extractors: {config.get('extractor_names')}")

                if result["success"]:
                    print("Status: SUCCESS")
                    print(f"MatchRunID: {result['match_result']['match_run_id']}")

                    if result["edge_result"] is not None:
                        print(f"EdgeRunID: {result['edge_result']['edge_run_id']}")
                else:
                    print("Status: FAILED")
                    print(result["error"])

                print("-" * 80)

            except Exception:
                error_message = traceback.format_exc()

                results.append({
                    "success": False,
                    "config": config,
                    "match_result": None,
                    "edge_result": None,
                    "error": error_message
                })

                print("\n❌ Future failed unexpectedly")
                print(error_message)

    success_count = sum(1 for r in results if r["success"])
    failed_count = len(results) - success_count

    print("\n" + "=" * 80)
    print("🏁 PARALLEL EXPERIMENT RUNNER FINISHED")
    print(f"Total: {len(results)}")
    print(f"Success: {success_count}")
    print(f"Failed: {failed_count}")
    print("=" * 80)

    return results


if __name__ == "__main__":
    run()