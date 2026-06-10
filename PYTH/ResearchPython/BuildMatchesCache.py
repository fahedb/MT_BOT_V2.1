from CandlesLoaders.HistoryLoader import load_history as load_history_data
from CandlesLoaders.CurrentLoader import load_history as load_current_data

from VectorBuilders.VectorPipeline import VectorPipeline
from VectorBuilders.ReturnsVectorBuilder import extract as returns_extract
from VectorBuilders.VolatilityVectorBuilder import extract as volatility_extract
from VectorBuilders.CandleStructureVectorBuilder import extract as candle_structure_extract
from VectorBuilders.H1TrendVectorBuilder import extract as h1_trend_extract
from VectorBuilders.RangeLocationVectorBuilder import extract as range_location_extract
from VectorBuilders.EmaSegmentShapeVectorBuilder import extract as ema_segment_shape_vec_extract
from VectorBuilders.RsiSegmentShapeVectorBuilder import extract as rsi_segment_shape_vec_extract
from VectorBuilders.RsiExtremeVectorBuilder import extract as rsi_extreme_extract
from VectorBuilders.EmaDirectionVectorBuilder import extract as ema_direction_extract
from VectorBuilders.EmaDistanceVectorBuilder import extract as ema_distance_extract

from VectorSearch.VectorSearchEngine import search_all_fast
from VectorSearch.LayeredVectorSearchEngine import search_all_layered_filter_only

from sqlalchemy import create_engine, text

import os
import json
import traceback


CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

MATCHES_DIR = "Matches"

DEFAULT_WINDOW_SIZE = 30
DEFAULT_TOP_K = 10
DEFAULT_DISTANCE_THRESHOLD = 0.8
DEFAULT_NORMALIZATION_METHOD = "feature_axis_0"


# =========================
# DEFAULT MANUAL CONFIG - REGULAR SEARCH
# =========================
EXTRACTORS_CONFIG = [
  ("VolatilityVectorBuilder", volatility_extract),
  ("CandleStructureVectorBuilder", candle_structure_extract),
  ("H1TrendVectorBuilder", h1_trend_extract),
  ("RsiExtremeVectorBuilder", rsi_extreme_extract),
  ("EmaDistanceVectorBuilder", ema_distance_extract),
]


# =========================
# DEFAULT MANUAL CONFIG - LAYERED SEARCH
# =========================
LAYERED_CONFIG = [
    {
        "name": "ema_shape",
        "extractors_config": [
            ("EmaSegmentShapeVectorBuilder", ema_segment_shape_vec_extract),
        ],
        "keep_top_n": 1500,
    },
    {
        "name": "base",
        "extractors_config": [
            ("VolatilityVectorBuilder", volatility_extract),
            ("CandleStructureVectorBuilder", candle_structure_extract),
            ("H1TrendVectorBuilder", h1_trend_extract),
        ],
        "keep_top_n": 10,
    },
]


# =========================
# DB HELPERS
# =========================
def create_engine_instance():
    return create_engine(CONN_STR)


def create_match_run(
    extractor_names,
    vector_size,
    window_size,
    top_k,
    distance_threshold,
    normalization_method,
    extra_config=None
):
    engine = create_engine_instance()

    extractors_json = json.dumps(extractor_names)

    config = {
        "extractors": extractor_names,
        "vector_size": vector_size,
        "window_size": window_size,
        "top_k": top_k,
        "distance_threshold": distance_threshold,
        "normalization_method": normalization_method
    }

    if extra_config is not None:
        config.update(extra_config)

    config_json = json.dumps(config)

    query = text("""
        INSERT INTO dbo.VectorMatchRuns
        (
            ExtractorsJson,
            ConfigJson,
            VectorSize,
            WindowSize,
            TopK,
            DistanceThreshold,
            NormalizationMethod
        )
        OUTPUT INSERTED.MatchRunID
        VALUES
        (
            :ExtractorsJson,
            :ConfigJson,
            :VectorSize,
            :WindowSize,
            :TopK,
            :DistanceThreshold,
            :NormalizationMethod
        )
    """)

    with engine.begin() as conn:
        match_run_id = conn.execute(query, {
            "ExtractorsJson": extractors_json,
            "ConfigJson": config_json,
            "VectorSize": vector_size,
            "WindowSize": window_size,
            "TopK": top_k,
            "DistanceThreshold": distance_threshold,
            "NormalizationMethod": normalization_method
        }).scalar()

    return int(match_run_id)


def update_match_run_file(match_run_id, matches_file):
    engine = create_engine_instance()

    query = text("""
        UPDATE dbo.VectorMatchRuns
        SET MatchesFile = :MatchesFile
        WHERE MatchRunID = :MatchRunID
    """)

    with engine.begin() as conn:
        conn.execute(query, {
            "MatchRunID": match_run_id,
            "MatchesFile": matches_file
        })


def update_match_run_status(match_run_id, status, progress_percent=None, error_message=None):
    """
    This function is optional.
    It will work only if you added these columns to dbo.VectorMatchRuns:
        Status, ProgressPercent, StartedAt, FinishedAt, ErrorMessage
    """

    engine = create_engine_instance()

    if status == "RUNNING":
        query = text("""
            UPDATE dbo.VectorMatchRuns
            SET 
                Status = :Status,
                ProgressPercent = :ProgressPercent,
                StartedAt = SYSUTCDATETIME(),
                ErrorMessage = NULL
            WHERE MatchRunID = :MatchRunID
        """)
    elif status in ("COMPLETED", "FAILED"):
        query = text("""
            UPDATE dbo.VectorMatchRuns
            SET 
                Status = :Status,
                ProgressPercent = :ProgressPercent,
                FinishedAt = SYSUTCDATETIME(),
                ErrorMessage = :ErrorMessage
            WHERE MatchRunID = :MatchRunID
        """)
    else:
        query = text("""
            UPDATE dbo.VectorMatchRuns
            SET 
                Status = :Status,
                ProgressPercent = :ProgressPercent,
                ErrorMessage = :ErrorMessage
            WHERE MatchRunID = :MatchRunID
        """)

    with engine.begin() as conn:
        conn.execute(query, {
            "MatchRunID": match_run_id,
            "Status": status,
            "ProgressPercent": progress_percent,
            "ErrorMessage": error_message
        })


# =========================
# MATCHES OUTPUT
# =========================
def build_matches_output(
    matches,
    match_run_id,
    extractor_names,
    vector_size,
    window_size,
    top_k,
    distance_threshold,
    normalization_method
):
    output = {
        "metadata": {
            "match_run_id": match_run_id,
            "extractors": extractor_names,
            "vector_size": vector_size,
            "window_size": window_size,
            "top_k": top_k,
            "distance_threshold": distance_threshold,
            "normalization_method": normalization_method
        },
        "matches": {}
    }

    for row in matches:
        curr = int(row["CurrentTP"])

        match_item = {
            "MatchTP": int(row["MatchTP"]),
            "Distance": float(row["Distance"]),
            "Rank": int(row["Rank"])
        }

        # Optional data for layered search.
        # EdgeFinder can ignore this safely.
        if "LayerDistances" in row:
            match_item["LayerDistances"] = row["LayerDistances"]

        output["matches"].setdefault(str(curr), []).append(match_item)

    return output


def save_matches_file(match_run_id, output):
    os.makedirs(MATCHES_DIR, exist_ok=True)

    matches_file = os.path.join(
        MATCHES_DIR,
        f"matches_run_{match_run_id}.json"
    )

    with open(matches_file, "w") as f:
        json.dump(output, f)

    return matches_file


# =========================
# REGULAR MATCH BUILDER
# =========================
def build_matches_for_config(
    extractors_config,
    window_size=DEFAULT_WINDOW_SIZE,
    top_k=DEFAULT_TOP_K,
    distance_threshold=DEFAULT_DISTANCE_THRESHOLD,
    normalization_method=DEFAULT_NORMALIZATION_METHOD,
    use_status_tracking=False
):
    """
    Regular one-layer vector search.

    Parameters:
        extractors_config:
            [
                ("VolatilityVectorBuilder", volatility_extract),
                ("CandleStructureVectorBuilder", candle_structure_extract),
            ]
    """

    print("\n=== BUILD MATCHES FOR CONFIG ===")

    if not extractors_config:
        raise ValueError("extractors_config is empty ❌")

    extractor_names = [name for name, _ in extractors_config]
    extractors = [extractor for _, extractor in extractors_config]

    print(f"Extractors: {extractor_names}")

    match_run_id = None

    try:
        candles_hist, indicators_hist = load_history_data()
        candles_curr, indicators_curr = load_current_data()

        pipeline = VectorPipeline(
            extractors=extractors,
            window_size=window_size
        )

        vectors_hist = pipeline.build_all_vectors(candles_hist, indicators_hist)
        vectors_curr = pipeline.build_all_vectors(candles_curr, indicators_curr)

        if len(vectors_hist) == 0:
            raise ValueError("No history vectors created ❌")

        if len(vectors_curr) == 0:
            raise ValueError("No current vectors created ❌")

        vector_size = len(next(iter(vectors_hist.values())))

        match_run_id = create_match_run(
            extractor_names=extractor_names,
            vector_size=vector_size,
            window_size=window_size,
            top_k=top_k,
            distance_threshold=distance_threshold,
            normalization_method=normalization_method,
            extra_config={
                "search_type": "regular"
            }
        )

        print(f"Created MatchRunID: {match_run_id}")

        if use_status_tracking:
            update_match_run_status(
                match_run_id=match_run_id,
                status="RUNNING",
                progress_percent=0
            )

        matches = search_all_fast(
            vectors_current=vectors_curr,
            vectors_history=vectors_hist,
            top_k=top_k,
            distance_threshold=distance_threshold
        )

        output = build_matches_output(
            matches=matches,
            match_run_id=match_run_id,
            extractor_names=extractor_names,
            vector_size=vector_size,
            window_size=window_size,
            top_k=top_k,
            distance_threshold=distance_threshold,
            normalization_method=normalization_method
        )

        output["metadata"]["search_type"] = "regular"

        matches_file = save_matches_file(
            match_run_id=match_run_id,
            output=output
        )

        update_match_run_file(
            match_run_id=match_run_id,
            matches_file=matches_file
        )

        if use_status_tracking:
            update_match_run_status(
                match_run_id=match_run_id,
                status="COMPLETED",
                progress_percent=100
            )

        print(f"✅ Saved {matches_file}")

        return {
            "match_run_id": match_run_id,
            "matches_file": matches_file,
            "extractors": extractor_names,
            "vector_size": vector_size,
            "matches_count": len(matches)
        }

    except Exception as e:
        error_message = traceback.format_exc()

        print("❌ Build matches failed")
        print(error_message)

        if use_status_tracking and match_run_id is not None:
            update_match_run_status(
                match_run_id=match_run_id,
                status="FAILED",
                progress_percent=None,
                error_message=error_message
            )

        raise e


# =========================
# LAYERED FILTER-ONLY MATCH BUILDER
# =========================
def build_layered_matches_for_config(
    layers_config,
    window_size=DEFAULT_WINDOW_SIZE,
    top_k=DEFAULT_TOP_K,
    distance_threshold=DEFAULT_DISTANCE_THRESHOLD,
    normalization_method="layered_filter_only",
    use_status_tracking=False
):
    """
    Builds matches using filter-only layered search.

    Each layer receives candidates from the previous layer,
    ranks them by its own vector space,
    and keeps keep_top_n candidates.

    Example:
        layers_config = [
            {
                "name": "base",
                "extractors_config": [
                    ("VolatilityVectorBuilder", volatility_extract),
                    ("CandleStructureVectorBuilder", candle_structure_extract),
                    ("H1TrendVectorBuilder", h1_trend_extract),
                ],
                "keep_top_n": 500,
            },
            {
                "name": "rsi_shape",
                "extractors_config": [
                    ("RsiSegmentShapeVectorBuilder", rsi_segment_shape_vec_extract),
                ],
                "keep_top_n": 50,
            },
            {
                "name": "ema_shape",
                "extractors_config": [
                    ("EmaSegmentShapeVectorBuilder", ema_segment_shape_vec_extract),
                ],
                "keep_top_n": 10,
            },
        ]
    """

    print("\n=== BUILD LAYERED MATCHES FOR CONFIG ===")

    if not layers_config:
        raise ValueError("layers_config is empty ❌")

    match_run_id = None

    try:
        candles_hist, indicators_hist = load_history_data()
        candles_curr, indicators_curr = load_current_data()

        vectors_history_by_layer = {}
        vectors_current_by_layer = {}

        extractor_names_flat = []
        layers_metadata = []
        total_vector_size = 0

        # =========================
        # BUILD VECTORS PER LAYER
        # =========================
        for layer in layers_config:
            layer_name = layer.get("name")
            extractors_config = layer.get("extractors_config")
            keep_top_n = int(layer.get("keep_top_n", top_k))

            if not layer_name:
                raise ValueError("Layer missing name ❌")

            if not extractors_config:
                raise ValueError(f"Layer {layer_name} has empty extractors_config ❌")

            layer_extractor_names = [
                name for name, _ in extractors_config
            ]

            layer_extractors = [
                extractor for _, extractor in extractors_config
            ]

            print("\n" + "=" * 80)
            print(f"BUILDING LAYER: {layer_name}")
            print(f"Extractors: {layer_extractor_names}")
            print(f"KeepTopN: {keep_top_n}")
            print("=" * 80)

            pipeline = VectorPipeline(
                extractors=layer_extractors,
                window_size=window_size
            )

            vectors_hist_layer = pipeline.build_all_vectors(
                candles_hist,
                indicators_hist
            )

            vectors_curr_layer = pipeline.build_all_vectors(
                candles_curr,
                indicators_curr
            )

            if len(vectors_hist_layer) == 0:
                raise ValueError(f"No history vectors created for layer {layer_name} ❌")

            if len(vectors_curr_layer) == 0:
                raise ValueError(f"No current vectors created for layer {layer_name} ❌")

            layer_vector_size = len(next(iter(vectors_hist_layer.values())))

            vectors_history_by_layer[layer_name] = vectors_hist_layer
            vectors_current_by_layer[layer_name] = vectors_curr_layer

            total_vector_size += layer_vector_size

            extractor_names_flat.extend([
                f"{layer_name}::{name}"
                for name in layer_extractor_names
            ])

            layers_metadata.append({
                "name": layer_name,
                "extractors": layer_extractor_names,
                "vector_size": layer_vector_size,
                "keep_top_n": keep_top_n
            })

        # =========================
        # CREATE MATCH RUN
        # =========================
        match_run_id = create_match_run(
            extractor_names=extractor_names_flat,
            vector_size=total_vector_size,
            window_size=window_size,
            top_k=top_k,
            distance_threshold=distance_threshold,
            normalization_method=normalization_method,
            extra_config={
                "search_type": "layered_filter_only",
                "layers": layers_metadata
            }
        )

        print(f"Created MatchRunID: {match_run_id}")

        if use_status_tracking:
            update_match_run_status(
                match_run_id=match_run_id,
                status="RUNNING",
                progress_percent=0
            )

        # =========================
        # SEARCH
        # =========================
        search_layers_config = []

        for layer in layers_config:
            search_layers_config.append({
                "name": layer["name"],
                "keep_top_n": int(layer.get("keep_top_n", top_k))
            })

        matches = search_all_layered_filter_only(
            vectors_current_by_layer=vectors_current_by_layer,
            vectors_history_by_layer=vectors_history_by_layer,
            layers_config=search_layers_config,
            final_top_k=top_k
        )

        # =========================
        # OUTPUT
        # =========================
        output = build_matches_output(
            matches=matches,
            match_run_id=match_run_id,
            extractor_names=extractor_names_flat,
            vector_size=total_vector_size,
            window_size=window_size,
            top_k=top_k,
            distance_threshold=distance_threshold,
            normalization_method=normalization_method
        )

        output["metadata"]["search_type"] = "layered_filter_only"
        output["metadata"]["layers"] = layers_metadata

        matches_file = save_matches_file(
            match_run_id=match_run_id,
            output=output
        )

        update_match_run_file(
            match_run_id=match_run_id,
            matches_file=matches_file
        )

        if use_status_tracking:
            update_match_run_status(
                match_run_id=match_run_id,
                status="COMPLETED",
                progress_percent=100
            )

        print(f"✅ Saved {matches_file}")

        return {
            "match_run_id": match_run_id,
            "matches_file": matches_file,
            "extractors": extractor_names_flat,
            "layers": layers_metadata,
            "vector_size": total_vector_size,
            "matches_count": len(matches)
        }

    except Exception as e:
        error_message = traceback.format_exc()

        print("❌ Build layered matches failed")
        print(error_message)

        if use_status_tracking and match_run_id is not None:
            update_match_run_status(
                match_run_id=match_run_id,
                status="FAILED",
                progress_percent=None,
                error_message=error_message
            )

        raise e


# =========================
# MANUAL RUN
# =========================
def run_regular():
    return build_matches_for_config(
        extractors_config=EXTRACTORS_CONFIG,
        window_size=DEFAULT_WINDOW_SIZE,
        top_k=DEFAULT_TOP_K,
        distance_threshold=DEFAULT_DISTANCE_THRESHOLD,
        normalization_method=DEFAULT_NORMALIZATION_METHOD,
        use_status_tracking=False
    )


def run_layered():
    return build_layered_matches_for_config(
        layers_config=LAYERED_CONFIG,
        window_size=DEFAULT_WINDOW_SIZE,
        top_k=DEFAULT_TOP_K,
        distance_threshold=DEFAULT_DISTANCE_THRESHOLD,
        normalization_method="layered_filter_only",
        use_status_tracking=False
    )


def run():
    # Change this if you want the old regular mode.
    #return run_layered()
    return run_regular()


if __name__ == "__main__":
    run()