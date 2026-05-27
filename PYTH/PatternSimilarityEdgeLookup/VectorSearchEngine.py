import numpy as np

TOP_K = 20
DIST_THRESHOLD = 0.8
TOP_K = 10

# =========================
# PREPARE HISTORY (ONCE)
# =========================
def prepare_history_matrix(history_vectors):

    print("Preparing history matrix...")

    history_tps = np.array(list(history_vectors.keys()))
    history_matrix = np.array(list(history_vectors.values()), dtype=np.float32)

    # normalize once
    mean = history_matrix.mean(axis=1, keepdims=True)
    std = history_matrix.std(axis=1, keepdims=True)

    history_matrix = (history_matrix - mean) / (std + 1e-6)

    print(f"History matrix shape: {history_matrix.shape}")

    return history_tps, history_matrix


# =========================
# FIND TOP-K (FAST)
# =========================
def find_top_k_fast(current_tp, current_vec, history_tps, history_matrix):

    mask = history_tps < current_tp

    if not np.any(mask):
        return []

    filtered_tps = history_tps[mask]
    filtered_matrix = history_matrix[mask]

    curr = (current_vec - np.mean(current_vec)) / (np.std(current_vec) + 1e-6)

    distances = np.mean(np.abs(filtered_matrix - curr), axis=1)

    # 🔥 פילטר חזק
    valid_mask = distances < DIST_THRESHOLD

    if not np.any(valid_mask):
        return []

    distances = distances[valid_mask]
    filtered_tps = filtered_tps[valid_mask]

    # TOP K קטן יותר
    k = min(TOP_K, len(distances))
    idx = np.argpartition(distances, k)[:k]
    idx = idx[np.argsort(distances[idx])]

    return list(zip(filtered_tps[idx], distances[idx]))


# =========================
# MAIN ENGINE
# =========================
def search_all_fast(vectors_current, vectors_history):

    print("\n=== FAST Vector Search Engine ===")

    history_tps, history_matrix = prepare_history_matrix(vectors_history)

    all_results = []

    for i, (curr_tp, curr_vec) in enumerate(vectors_current.items()):

        top_matches = find_top_k_fast(curr_tp, curr_vec, history_tps, history_matrix)

        for rank, (match_tp, dist) in enumerate(top_matches):

            all_results.append({
                "CurrentTP": curr_tp,
                "MatchTP": int(match_tp),
                "Distance": float(dist),
                "Rank": rank + 1
            })

        if i % 10 == 0:
            print(f"Processed {i}")

    print(f"Total matches found: {len(all_results)}")

    return all_results