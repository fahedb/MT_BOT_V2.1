def build_window_size_grid_configs(
    base_extractors_config,
    window_size_values=None,
    top_k=10,
    distance_threshold=0.8,
    normalization_method="feature_axis_0",
    grid_size="small",
    limit=None
):
    """
    Builds experiment configs for window_size.

    grid_size options:
        "tiny", "small", "medium", "large"

    You can override window_size_values manually.
    """

    if window_size_values is None:
        if grid_size == "tiny":
            window_size_values = [50]
        elif grid_size == "small":
            window_size_values = [30, 50, 80]
        elif grid_size == "medium":
            window_size_values = [20, 30, 50, 80]
        elif grid_size == "large":
            window_size_values = [10, 20, 30, 50, 80, 100]
        else:
            raise ValueError(f"Unknown grid_size: {grid_size}")

    extractor_names = [name for name, _ in base_extractors_config]

    configs = []

    for window_size in window_size_values:

        configs.append({
            "experiment_type": "window_size_grid",
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