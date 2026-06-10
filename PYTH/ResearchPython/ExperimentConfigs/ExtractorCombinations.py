from itertools import combinations

from VectorBuilders.ReturnsVectorBuilder import extract as returns_extract
from VectorBuilders.VolatilityVectorBuilder import extract as volatility_extract
from VectorBuilders.CandleStructureVectorBuilder import extract as candle_structure_extract
from VectorBuilders.H1TrendVectorBuilder import extract as h1_trend_extract
from VectorBuilders.RangeLocationVectorBuilder import extract as range_location_extract

from VectorBuilders.RsiDirectionVectorBuilder import extract as rsi_direction_extract
from VectorBuilders.RsiLevelVectorBuilder import extract as rsi_level_extract
from VectorBuilders.RsiExtremeVectorBuilder import extract as rsi_extreme_extract
from VectorBuilders.EmaDirectionVectorBuilder import extract as ema_direction_extract
from VectorBuilders.EmaDistanceVectorBuilder import extract as ema_distance_extract

ALL_EXTRACTORS = [
    ("ReturnsVectorBuilder", returns_extract),
    ("VolatilityVectorBuilder", volatility_extract),
    ("CandleStructureVectorBuilder", candle_structure_extract),
    ("H1TrendVectorBuilder", h1_trend_extract),

    ("RangeLocationVectorBuilder", range_location_extract),

    ("RsiDirectionVectorBuilder", rsi_direction_extract),
    ("RsiLevelVectorBuilder", rsi_level_extract),
    ("RsiExtremeVectorBuilder", rsi_extreme_extract),

    ("EmaDirectionVectorBuilder", ema_direction_extract),
    ("EmaDistanceVectorBuilder", ema_distance_extract),
]

def build_extractor_combination_configs(
    min_size=2,
    max_size=None,
    window_size=50,
    top_k=10,
    distance_threshold=0.8,
    normalization_method="feature_axis_0",
    limit=None,
    base_extractors_config=None,
    candidate_extractors_config=None,
    include_base_only=False
):
    """
    Builds experiment configs for extractor combinations.

    Modes:

    1. Regular mode:
        base_extractors_config=None
        candidate_extractors_config=None

        Builds combinations from ALL_EXTRACTORS.

    2. Base mode:
        base_extractors_config=[...]
        candidate_extractors_config=[...]

        Builds:
            base + combo(candidate extractors)

    Parameters:
        min_size / max_size:
            In regular mode:
                size of the full extractor combination.

            In base mode:
                size of the candidate combo added to the base.

        include_base_only:
            In base mode, also adds the base combination without additions.
    """

    configs = []

    # =========================
    # REGULAR MODE
    # =========================
    if base_extractors_config is None:

        source_extractors = (
            candidate_extractors_config
            if candidate_extractors_config is not None
            else ALL_EXTRACTORS
        )

        if max_size is None:
            max_size = len(source_extractors)

        for size in range(min_size, max_size + 1):
            for combo in combinations(source_extractors, size):

                extractor_names = [name for name, _ in combo]

                configs.append({
                    "experiment_type": "extractor_combination",
                    "extractors_config": list(combo),
                    "extractor_names": extractor_names,
                    "window_size": window_size,
                    "top_k": top_k,
                    "distance_threshold": distance_threshold,
                    "normalization_method": normalization_method,
                })

                if limit is not None and len(configs) >= limit:
                    return configs

        return configs

    # =========================
    # BASE MODE
    # =========================
    base_names = [name for name, _ in base_extractors_config]

    if candidate_extractors_config is None:
        base_name_set = set(base_names)

        candidate_extractors_config = [
            item for item in ALL_EXTRACTORS
            if item[0] not in base_name_set
        ]

    if include_base_only:
        configs.append({
            "experiment_type": "base_plus_combinations",
            "extractors_config": list(base_extractors_config),
            "extractor_names": base_names,
            "window_size": window_size,
            "top_k": top_k,
            "distance_threshold": distance_threshold,
            "normalization_method": normalization_method,
        })

        if limit is not None and len(configs) >= limit:
            return configs

    if max_size is None:
        max_size = len(candidate_extractors_config)

    for add_size in range(min_size, max_size + 1):
        for added_combo in combinations(candidate_extractors_config, add_size):

            full_combo = list(base_extractors_config) + list(added_combo)
            extractor_names = [name for name, _ in full_combo]

            configs.append({
                "experiment_type": "base_plus_combinations",
                "extractors_config": full_combo,
                "extractor_names": extractor_names,
                "window_size": window_size,
                "top_k": top_k,
                "distance_threshold": distance_threshold,
                "normalization_method": normalization_method,
                "base_extractors": base_names,
                "added_extractors": [name for name, _ in added_combo],
            })

            if limit is not None and len(configs) >= limit:
                return configs

    return configs