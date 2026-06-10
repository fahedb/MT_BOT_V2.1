import numpy as np


DEFAULT_TOP_K = 10
DEFAULT_DISTANCE_THRESHOLD = 0.8


# =========================
# PREPARE HISTORY (ONCE)
# =========================
def prepare_history_matrix(history_vectors):

    print("Preparing history matrix...")

    history_tps = np.array(list(history_vectors.keys()))
    history_matrix = np.array(list(history_vectors.values()), dtype=np.float32)

    feature_mean = history_matrix.mean(axis=0, keepdims=True)
    feature_std = history_matrix.std(axis=0, keepdims=True)

    history_matrix_norm = (history_matrix - feature_mean) / (feature_std + 1e-6)

    print(f"History matrix shape: {history_matrix_norm.shape}")

    return history_tps, history_matrix_norm, feature_mean, feature_std


# =========================
# FIND TOP-K (FAST)
# =========================
def find_top_k_fast(
    current_tp,
    current_vec,
    history_tps,
    history_matrix,
    feature_mean,
    feature_std,
    top_k=DEFAULT_TOP_K,
    distance_threshold=DEFAULT_DISTANCE_THRESHOLD
):

    mask = history_tps < current_tp

    if not np.any(mask):
        return []

    filtered_tps = history_tps[mask]
    filtered_matrix = history_matrix[mask]

    current_vec = np.array(current_vec, dtype=np.float32)

    # Normalize current vector using HISTORY feature statistics
    curr_norm = (current_vec - feature_mean[0]) / (feature_std[0] + 1e-6)

    # Mean absolute distance between normalized feature vectors
    distances = np.mean(np.abs(filtered_matrix - curr_norm), axis=1)

    # Keep only matches close enough
    valid_mask = distances < distance_threshold

    if not np.any(valid_mask):
        return []

    distances = distances[valid_mask]
    filtered_tps = filtered_tps[valid_mask]

    k = min(top_k, len(distances))

    if k <= 0:
        return []

    idx = np.argpartition(distances, k - 1)[:k]
    idx = idx[np.argsort(distances[idx])]

    return list(zip(filtered_tps[idx], distances[idx]))


# =========================
# MAIN ENGINE
# =========================
def search_all_fast(
    vectors_current,
    vectors_history,
    top_k=DEFAULT_TOP_K,
    distance_threshold=DEFAULT_DISTANCE_THRESHOLD
):

    print("\n=== FAST Vector Search Engine ===")
    print(f"TOP_K: {top_k}")
    print(f"DISTANCE_THRESHOLD: {distance_threshold}")

    history_tps, history_matrix, feature_mean, feature_std = prepare_history_matrix(vectors_history)

    all_results = []

    total = len(vectors_current)

    for i, (curr_tp, curr_vec) in enumerate(vectors_current.items()):

        top_matches = find_top_k_fast(
            current_tp=curr_tp,
            current_vec=curr_vec,
            history_tps=history_tps,
            history_matrix=history_matrix,
            feature_mean=feature_mean,
            feature_std=feature_std,
            top_k=top_k,
            distance_threshold=distance_threshold
        )

        for rank, (match_tp, dist) in enumerate(top_matches):

            all_results.append({
                "CurrentTP": int(curr_tp),
                "MatchTP": int(match_tp),
                "Distance": float(dist),
                "Rank": rank + 1
            })

        if i % 10 == 0:
            print(f"Processed {i}/{total}")

    print(f"Total matches found: {len(all_results)}")

    return all_results