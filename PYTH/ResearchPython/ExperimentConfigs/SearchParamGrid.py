from itertools import product


def build_search_param_grid_configs(
    base_extractors_config,
    top_k_values=None,
    distance_threshold_values=None,
    window_size=50,
    normalization_method="feature_axis_0",
    grid_size="small",
    limit=None
):
    """
    Builds experiment configs for search parameters:
        - top_k
        - distance_threshold

    grid_size options:
        "tiny", "small", "medium", "large"

    You can override top_k_values and distance_threshold_values manually.
    """

    if top_k_values is None:
        if grid_size == "tiny":
            top_k_values = [10]
        elif grid_size == "small":
            top_k_values = [5, 10, 20]
        elif grid_size == "medium":
            top_k_values = [3, 5, 10, 20, 50]
        elif grid_size == "large":
            top_k_values = [1, 3, 5, 10, 20, 30, 50, 100]
        else:
            raise ValueError(f"Unknown grid_size: {grid_size}")

    if distance_threshold_values is None:
        if grid_size == "tiny":
            distance_threshold_values = [0.8]
        elif grid_size == "small":
            distance_threshold_values = [0.6, 0.8, 1.0]
        elif grid_size == "medium":
            distance_threshold_values = [0.4, 0.6, 0.8, 1.0, 1.2]
        elif grid_size == "large":
            distance_threshold_values = [0.3, 0.4, 0.5, 0.6, 0.8, 1.0, 1.2, 1.5]
        else:
            raise ValueError(f"Unknown grid_size: {grid_size}")

    extractor_names = [name for name, _ in base_extractors_config]

    configs = []

    for top_k, distance_threshold in product(top_k_values, distance_threshold_values):

        configs.append({
            "experiment_type": "search_param_grid",
            "extractors_config": base_extractors_config,
            "extractor_names": extractor_names,
            "window_size": window_size,
            "top_k": top_k,
            "distance_threshold": distance_threshold,
            "normalization_method": normalization_method,
        })

        if limit is not None and len(configs) >= limit:
            return configs

    return configs