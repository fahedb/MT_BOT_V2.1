import numpy as np


def prepare_history_matrix(vectors):
    print("Preparing layered history matrix...")

    history_tps = np.array(list(vectors.keys()))
    history_matrix = np.array(list(vectors.values()), dtype=np.float32)

    feature_mean = history_matrix.mean(axis=0, keepdims=True)
    feature_std = history_matrix.std(axis=0, keepdims=True)

    history_matrix_norm = (history_matrix - feature_mean) / (feature_std + 1e-6)

    tp_to_index = {
        int(tp): i
        for i, tp in enumerate(history_tps)
    }

    print(f"Layer history matrix shape: {history_matrix_norm.shape}")

    return {
        "history_tps": history_tps,
        "history_matrix": history_matrix_norm,
        "feature_mean": feature_mean,
        "feature_std": feature_std,
        "tp_to_index": tp_to_index,
    }


def normalize_current_vector(current_vec, feature_mean, feature_std):
    current_vec = np.array(current_vec, dtype=np.float32)
    return (current_vec - feature_mean[0]) / (feature_std[0] + 1e-6)


def calc_layer_distance(current_vec_norm, history_vec_norm):
    return float(np.mean(np.abs(history_vec_norm - current_vec_norm)))


def search_first_layer(
    current_tp,
    current_vec,
    layer_history_data,
    keep_top_n
):
    history_tps = layer_history_data["history_tps"]
    history_matrix = layer_history_data["history_matrix"]
    feature_mean = layer_history_data["feature_mean"]
    feature_std = layer_history_data["feature_std"]

    mask = history_tps < current_tp

    if not np.any(mask):
        return []

    filtered_tps = history_tps[mask]
    filtered_matrix = history_matrix[mask]

    curr_norm = normalize_current_vector(
        current_vec=current_vec,
        feature_mean=feature_mean,
        feature_std=feature_std
    )

    distances = np.mean(np.abs(filtered_matrix - curr_norm), axis=1)

    n = min(int(keep_top_n), len(distances))

    if n <= 0:
        return []

    idx = np.argpartition(distances, n - 1)[:n]
    idx = idx[np.argsort(distances[idx])]

    results = []

    for rank_idx in idx:
        results.append({
            "MatchTP": int(filtered_tps[rank_idx]),
            "Distance": float(distances[rank_idx]),
            "LayerDistances": {}
        })

    return results


def filter_candidates_by_layer(
    current_vec,
    candidates,
    layer_history_data,
    keep_top_n,
    layer_name
):
    feature_mean = layer_history_data["feature_mean"]
    feature_std = layer_history_data["feature_std"]
    history_matrix = layer_history_data["history_matrix"]
    tp_to_index = layer_history_data["tp_to_index"]

    curr_norm = normalize_current_vector(
        current_vec=current_vec,
        feature_mean=feature_mean,
        feature_std=feature_std
    )

    scored = []

    for candidate in candidates:
        match_tp = int(candidate["MatchTP"])

        hist_idx = tp_to_index.get(match_tp)

        if hist_idx is None:
            continue

        hist_vec_norm = history_matrix[hist_idx]

        layer_distance = calc_layer_distance(
            current_vec_norm=curr_norm,
            history_vec_norm=hist_vec_norm
        )

        new_candidate = {
            "MatchTP": match_tp,
            "Distance": float(layer_distance),
            "LayerDistances": dict(candidate.get("LayerDistances", {}))
        }

        new_candidate["LayerDistances"][layer_name] = float(layer_distance)

        scored.append(new_candidate)

    if len(scored) == 0:
        return []

    scored.sort(key=lambda x: x["Distance"])

    return scored[:int(keep_top_n)]


def search_all_layered_filter_only(
    vectors_current_by_layer,
    vectors_history_by_layer,
    layers_config,
    final_top_k
):
    """
    Filter-only layered search.

    Each layer receives candidates from the previous layer,
    ranks them only by its own distance,
    and keeps keep_top_n candidates.

    The final output is the final layer top_k.
    """

    print("\n=== FILTER-ONLY LAYERED VECTOR SEARCH ===")
    print(f"Layers count: {len(layers_config)}")
    print(f"Final TOP_K: {final_top_k}")

    if not layers_config:
        raise ValueError("layers_config is empty ❌")

    # =========================
    # PREPARE HISTORY MATRICES
    # =========================
    history_data_by_layer = {}

    for layer in layers_config:
        layer_name = layer["name"]

        print("\n" + "=" * 80)
        print(f"Preparing layer: {layer_name}")
        print("=" * 80)

        history_vectors = vectors_history_by_layer[layer_name]

        history_data_by_layer[layer_name] = prepare_history_matrix(history_vectors)

    # =========================
    # COMMON CURRENT TPS
    # =========================
    current_tp_sets = []

    for layer in layers_config:
        layer_name = layer["name"]
        current_tp_sets.append(set(vectors_current_by_layer[layer_name].keys()))

    common_current_tps = sorted(set.intersection(*current_tp_sets))

    print(f"\nCommon current TPs: {len(common_current_tps)}")

    all_results = []

    # =========================
    # SEARCH
    # =========================
    for i, curr_tp in enumerate(common_current_tps):

        candidates = None
        valid = True

        for layer_index, layer in enumerate(layers_config):
            layer_name = layer["name"]
            keep_top_n = layer.get("keep_top_n")

            if keep_top_n is None:
                keep_top_n = final_top_k

            current_vec = vectors_current_by_layer[layer_name].get(curr_tp)

            if current_vec is None:
                valid = False
                break

            layer_history_data = history_data_by_layer[layer_name]

            if layer_index == 0:
                candidates = search_first_layer(
                    current_tp=curr_tp,
                    current_vec=current_vec,
                    layer_history_data=layer_history_data,
                    keep_top_n=keep_top_n
                )

                for c in candidates:
                    c["LayerDistances"][layer_name] = float(c["Distance"])

            else:
                candidates = filter_candidates_by_layer(
                    current_vec=current_vec,
                    candidates=candidates,
                    layer_history_data=layer_history_data,
                    keep_top_n=keep_top_n,
                    layer_name=layer_name
                )

            if not candidates:
                valid = False
                break

        if not valid:
            continue

        final_candidates = candidates[:int(final_top_k)]

        for rank, candidate in enumerate(final_candidates, start=1):
            all_results.append({
                "CurrentTP": int(curr_tp),
                "MatchTP": int(candidate["MatchTP"]),
                "Distance": float(candidate["Distance"]),
                "Rank": int(rank),
                "LayerDistances": candidate.get("LayerDistances", {})
            })

        if i % 10 == 0:
            print(f"Processed {i}/{len(common_current_tps)}")

    print(f"Total layered matches found: {len(all_results)}")

    return all_results