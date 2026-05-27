import argparse

# YOUR MODULES
from MatchesExtractor import load_matches
from LoadHistoryCandles import load_candles
from LoadCurrentCandles import load_current_candles
from PatternBuilder import build_patterns
from SimilarityEngine import compute_similarity
from FutureSimilariryEngine import compute_future
from Analyzer import analyze


# =========================
# MAIN PIPELINE
# =========================
def run_pipeline(json_path):

    print("\n==============================")
    print("🚀 START PATTERN VALIDATION PIPELINE")
    print("==============================")

    # =========================
    # 1. LOAD MATCHES
    # =========================
    df_matches = load_matches(json_path)

    # =========================
    # 2. LOAD HISTORY
    # =========================
    df_history, map_history = load_candles()

    # =========================
    # 3. LOAD CURRENT
    # =========================
    df_current, map_current = load_current_candles()

    # =========================
    # 4. BUILD PATTERNS
    # =========================
    patterns = build_patterns(
        df_matches,
        df_current, map_current,
        df_history, map_history
    )

    if not patterns:
        print("❌ No valid patterns")
        return

    # =========================
    # 5. SIMILARITY
    # =========================
    similarity_results = compute_similarity(patterns)

    # =========================
    # 6. FUTURE
    # =========================
    future_results = compute_future(
        similarity_results,
        df_history, map_history,
        df_current, map_current
    )

    if not future_results:
        print("❌ No future results")
        return

    # =========================
    # 7. ANALYZER
    # =========================
    analyze(future_results)

    print("\n==============================")
    print("✅ PIPELINE FINISHED")
    print("==============================")


# =========================
# CLI ENTRY
# =========================
if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Pattern Validator Pipeline")
    parser.add_argument("json_path", type=str, help="Path to Matches JSON")

    args = parser.parse_args()

    run_pipeline(args.json_path)