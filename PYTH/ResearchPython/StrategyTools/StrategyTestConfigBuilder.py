import json

from sqlalchemy import create_engine, text


# =========================
# CONFIG
# =========================

CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)


DEFAULT_SIMULATOR_CONFIG = {
    "initial_balance": 10000.0,
    "symbol": "XAUUSD",
    "pip_size": 0.01,
    "money_per_pip": 1.0,
    "spread_pips": 20.0,
    "same_candle_policy": "sl_first",
}


DEFAULT_STRATEGY_CONFIG = {
    "tp_pips": None,  # None = use first_hit_pips from Edge config
    "sl_pips": None,  # None = use first_hit_pips from Edge config
    "lot_size": 1.0,

    "max_open_trades": 1,
    "block_reentry_same_candle": True,

    "max_candles": None,

    "save_trades_csv": True,
    "save_trades_json": True,
    "save_summary_json": True,
}


# =========================
# DB
# =========================

def create_engine_instance():
    return create_engine(CONN_STR)

def load_edge_run(edge_run_id):
    """
    Loads EdgeRun + its MatchRun from DB.

    Uses SQL aliases and row._mapping so we do not depend on fragile column indexes.
    """

    engine = create_engine_instance()

    query = text("""
        SELECT
            e.EdgeRunID AS EdgeRunID,
            e.MatchRunID AS MatchRunID,
            e.EvaluationConfigJson AS EvaluationConfigJson,

            e.DatasetRows AS DatasetRows,
            e.PredReturnCorr AS PredReturnCorr,
            e.DirectionAccuracy AS DirectionAccuracy,
            e.BaselineReturn AS BaselineReturn,

            e.Top50Accuracy AS Top50Accuracy,
            e.Top50Samples AS Top50Samples,
            e.Top50AvgReturn AS Top50AvgReturn,

            e.Top40Accuracy AS Top40Accuracy,
            e.Top40Samples AS Top40Samples,
            e.Top40AvgReturn AS Top40AvgReturn,

            e.Top30Accuracy AS Top30Accuracy,
            e.Top30Samples AS Top30Samples,
            e.Top30AvgReturn AS Top30AvgReturn,

            e.Top20Accuracy AS Top20Accuracy,
            e.Top20Samples AS Top20Samples,
            e.Top20AvgReturn AS Top20AvgReturn,

            e.Top10Accuracy AS Top10Accuracy,
            e.Top10Samples AS Top10Samples,
            e.Top10AvgReturn AS Top10AvgReturn,

            e.Top5Accuracy AS Top5Accuracy,
            e.Top5Samples AS Top5Samples,
            e.Top5AvgReturn AS Top5AvgReturn,

            e.Notes AS Notes,
            e.CreatedAt AS CreatedAt,

            e.AvgHistReturnCorr AS AvgHistReturnCorr,
            e.MedianHistReturnCorr AS MedianHistReturnCorr,

            e.Top50Threshold AS Top50Threshold,
            e.Top40Threshold AS Top40Threshold,
            e.Top30Threshold AS Top30Threshold,
            e.Top20Threshold AS Top20Threshold,
            e.Top10Threshold AS Top10Threshold,
            e.Top5Threshold AS Top5Threshold,

            m.MatchesFile AS MatchesFile
        FROM dbo.VectorEdgeRuns e
        INNER JOIN dbo.VectorMatchRuns m
            ON e.MatchRunID = m.MatchRunID
        WHERE e.EdgeRunID = :EdgeRunID
    """)

    with engine.begin() as conn:
        row = conn.execute(query, {
            "EdgeRunID": int(edge_run_id)
        }).fetchone()

    if row is None:
        raise ValueError(f"EdgeRunID not found: {edge_run_id}")

    row = row._mapping

    evaluation_config_raw = row["EvaluationConfigJson"]

    if evaluation_config_raw is None or evaluation_config_raw == "":
        raise ValueError(f"EdgeRunID={edge_run_id} has empty EvaluationConfigJson")

    evaluation_config = json.loads(evaluation_config_raw)

    match_run_id = int(row["MatchRunID"])
    matches_file = row["MatchesFile"]

    if matches_file is None or matches_file == "":
        matches_file = f"Matches/matches_run_{match_run_id}.json"

    return {
        "edge_run_id": int(row["EdgeRunID"]),
        "match_run_id": match_run_id,
        "evaluation_config": evaluation_config,

        "thresholds": {
            "Top50": row["Top50Threshold"],
            "Top40": row["Top40Threshold"],
            "Top30": row["Top30Threshold"],
            "Top20": row["Top20Threshold"],
            "Top10": row["Top10Threshold"],
            "Top5": row["Top5Threshold"],
        },

        "samples": {
            "Top50": row["Top50Samples"],
            "Top40": row["Top40Samples"],
            "Top30": row["Top30Samples"],
            "Top20": row["Top20Samples"],
            "Top10": row["Top10Samples"],
            "Top5": row["Top5Samples"],
        },

        "accuracies": {
            "Top50": row["Top50Accuracy"],
            "Top40": row["Top40Accuracy"],
            "Top30": row["Top30Accuracy"],
            "Top20": row["Top20Accuracy"],
            "Top10": row["Top10Accuracy"],
            "Top5": row["Top5Accuracy"],
        },

        "avg_returns": {
            "Top50": row["Top50AvgReturn"],
            "Top40": row["Top40AvgReturn"],
            "Top30": row["Top30AvgReturn"],
            "Top20": row["Top20AvgReturn"],
            "Top10": row["Top10AvgReturn"],
            "Top5": row["Top5AvgReturn"],
        },

        "dataset_rows": row["DatasetRows"],
        "direction_accuracy": row["DirectionAccuracy"],
        "pred_return_corr": row["PredReturnCorr"],
        "baseline_return": row["BaselineReturn"],

        "avg_hist_return_corr": row["AvgHistReturnCorr"],
        "median_hist_return_corr": row["MedianHistReturnCorr"],

        "notes": row["Notes"],
        "created_at": row["CreatedAt"],

        "matches_file": matches_file,
    }


# =========================
# HELPERS
# =========================

def apply_overrides(base_config, overrides):
    """
    Applies overrides while ignoring keys with None value.

    Example:
        {"tp_pips": None} will not override base_config.
    """

    if overrides is None:
        return dict(base_config)

    result = dict(base_config)

    for key, value in overrides.items():
        if value is not None:
            result[key] = value

    return result


def get_edge_threshold(edge_data, edge_level):
    threshold = edge_data["thresholds"].get(edge_level)

    if threshold is None:
        raise ValueError(
            f"{edge_level}Threshold is NULL for EdgeRunID={edge_data['edge_run_id']}. "
            f"Choose another edge_level. Available thresholds: {edge_data['thresholds']}"
        )

    return float(threshold)


def infer_signal_direction_method(evaluation_config):
    """
    Translates EdgeFinder config to SignalEngine direction method.

    If EdgeFinder used weighted first-hit prediction:
        use weighted_first_hit

    If EdgeFinder used regular first-hit prediction:
        use first_hit

    Otherwise:
        use pred_return
    """

    future_direction_method = evaluation_config.get("future_direction_method")

    if future_direction_method == "first_hit_pips":
        if evaluation_config.get("use_weighted_first_hit_prediction", False):
            return "weighted_first_hit"

        return "first_hit"

    return "pred_return"


def get_first_hit_pips_from_edge(evaluation_config, default_value=200):
    first_hit_pips = evaluation_config.get("first_hit_pips")

    if first_hit_pips is None:
        return int(default_value)

    return int(first_hit_pips)


# =========================
# BUILD CONFIGS
# =========================

def build_signal_config_from_edge(
    edge_data,
    edge_level,
    signal_overrides=None
):
    evaluation_config = edge_data["evaluation_config"]
    threshold = get_edge_threshold(edge_data, edge_level)

    signal_direction_method = infer_signal_direction_method(evaluation_config)

    signal_config = {
        # =========================
        # Basic filters
        # =========================
        "min_matches_count": 5,

        "min_direction_agreement": evaluation_config.get("min_direction_agreement"),
        "min_weighted_direction_agreement": evaluation_config.get("min_weighted_direction_agreement"),

        # This comes from selected Top level threshold
        "min_abs_pred_return": threshold,

        # =========================
        # Direction logic
        # =========================
        "signal_direction_method": signal_direction_method,

        # =========================
        # First-hit settings
        # =========================
        "future_direction_method": evaluation_config.get("future_direction_method", "return_sign"),
        "first_hit_pips": evaluation_config.get("first_hit_pips"),
        "first_hit_future_window": evaluation_config.get("first_hit_future_window"),
        "pip_size": evaluation_config.get("pip_size", 0.01),
        "same_candle_policy": evaluation_config.get("same_candle_policy", "noise"),

        "ignore_noise_in_first_hit_prediction": evaluation_config.get(
            "ignore_noise_in_first_hit_prediction",
            True
        ),

        "min_first_hit_agreement": evaluation_config.get("min_first_hit_agreement"),
        "min_weighted_first_hit_agreement": evaluation_config.get("min_weighted_first_hit_agreement"),
        "min_valid_first_hit_count": evaluation_config.get("min_valid_first_hit_count"),
        "max_first_hit_noise_ratio": evaluation_config.get("max_first_hit_noise_ratio"),

        # =========================
        # Distance filters
        # =========================
        "max_avg_distance": None,
        "max_min_distance": None,

        # =========================
        # Trade direction allow-list
        # =========================
        "allow_buy": True,
        "allow_sell": True,

        # =========================
        # Prediction settings
        # =========================
        "prediction_method": evaluation_config.get("prediction_method", "weighted_average"),
        "weighting_method": evaluation_config.get("weighting_method", "1_over_distance"),
        "future_stats_function": evaluation_config.get("future_stats_function", "compute_future_stats_default"),

        "contrarian": False,

        # =========================
        # Export
        # =========================
        "auto_export_signals": True,
        "signals_output_dir": "Signals",
        "signals_file_prefix": (
            f"signals_edge_{edge_data['edge_run_id']}_"
            f"match_run_{edge_data['match_run_id']}_"
            f"{edge_level.lower()}"
        ),
        "export_csv": True,
        "export_json": True,
        "include_timestamp": True,
    }

    return apply_overrides(signal_config, signal_overrides)


def build_strategy_config_from_edge(
    edge_data,
    strategy_overrides=None
):
    evaluation_config = edge_data["evaluation_config"]

    first_hit_pips = get_first_hit_pips_from_edge(
        evaluation_config,
        default_value=200
    )

    base_strategy_config = dict(DEFAULT_STRATEGY_CONFIG)

    # Default: strategy TP/SL uses the same pips as Edge first_hit_pips.
    if base_strategy_config["tp_pips"] is None:
        base_strategy_config["tp_pips"] = first_hit_pips

    if base_strategy_config["sl_pips"] is None:
        base_strategy_config["sl_pips"] = first_hit_pips

    return apply_overrides(base_strategy_config, strategy_overrides)


def build_simulator_config(
    simulator_overrides=None
):
    return apply_overrides(DEFAULT_SIMULATOR_CONFIG, simulator_overrides)


def build_run_metadata_from_edge(edge_data, edge_level):
    evaluation_config = edge_data["evaluation_config"]

    threshold = get_edge_threshold(edge_data, edge_level)

    return {
        "edge_run_id": edge_data["edge_run_id"],
        "edge_level": edge_level,
        "edge_threshold": threshold,

        "edge_config_name": evaluation_config.get("name"),
        "edge_first_hit_pips": evaluation_config.get("first_hit_pips"),
        "edge_first_hit_future_window": evaluation_config.get("first_hit_future_window"),
        "edge_future_direction_method": evaluation_config.get("future_direction_method"),

        "edge_dataset_rows": edge_data.get("dataset_rows"),
        "edge_direction_accuracy": edge_data.get("direction_accuracy"),
        "edge_pred_return_corr": edge_data.get("pred_return_corr"),

        "edge_level_samples": edge_data["samples"].get(edge_level),
        "edge_level_accuracy": edge_data["accuracies"].get(edge_level),
        "edge_level_avg_return": edge_data["avg_returns"].get(edge_level),
    }


def build_strategy_test_payload_from_edge(
    edge_run_id,
    edge_level="Top10",
    simulator_overrides=None,
    signal_overrides=None,
    strategy_overrides=None,
):
    """
    Main function used by RunStrategyTest.

    Example:
        payload = build_strategy_test_payload_from_edge(
            edge_run_id=390,
            edge_level="Top10",
            simulator_overrides={
                "spread_pips": 20.0,
                "same_candle_policy": "sl_first",
            },
            strategy_overrides={
                "tp_pips": None,
                "sl_pips": None,
                "max_open_trades": 1,
            },
            signal_overrides={
                "allow_buy": True,
                "allow_sell": True,
            },
        )
    """

    edge_data = load_edge_run(edge_run_id)

    signal_config = build_signal_config_from_edge(
        edge_data=edge_data,
        edge_level=edge_level,
        signal_overrides=signal_overrides
    )

    strategy_config = build_strategy_config_from_edge(
        edge_data=edge_data,
        strategy_overrides=strategy_overrides
    )

    simulator_config = build_simulator_config(
        simulator_overrides=simulator_overrides
    )

    run_metadata = build_run_metadata_from_edge(
        edge_data=edge_data,
        edge_level=edge_level
    )

    payload = {
        "edge_run_id": edge_data["edge_run_id"],
        "edge_level": edge_level,
        "edge_threshold": get_edge_threshold(edge_data, edge_level),

        "match_run_id": edge_data["match_run_id"],
        "matches_file": edge_data["matches_file"],

        "evaluation_config": edge_data["evaluation_config"],

        "simulator_config": simulator_config,
        "signal_config": signal_config,
        "strategy_config": strategy_config,

        "run_metadata": run_metadata,
    }

    print_payload_summary(payload)

    return payload


# =========================
# DEBUG PRINT
# =========================

def print_payload_summary(payload):
    print("\n=== STRATEGY TEST PAYLOAD FROM EDGE ===")
    print(f"EdgeRunID: {payload['edge_run_id']}")
    print(f"EdgeLevel: {payload['edge_level']}")
    print(f"EdgeThreshold: {payload['edge_threshold']}")
    print(f"MatchRunID: {payload['match_run_id']}")
    print(f"MatchesFile: {payload['matches_file']}")

    evaluation_config = payload["evaluation_config"]
    print(f"EdgeConfigName: {evaluation_config.get('name')}")
    print(f"FutureDirectionMethod: {evaluation_config.get('future_direction_method')}")
    print(f"FirstHitPips: {evaluation_config.get('first_hit_pips')}")
    print(f"FirstHitFutureWindow: {evaluation_config.get('first_hit_future_window')}")

    print("\nSimulatorConfig:")
    print(payload["simulator_config"])

    print("\nSignalConfig:")
    print(payload["signal_config"])

    print("\nStrategyConfig:")
    print(payload["strategy_config"])
    print("=" * 80)