import pandas as pd
import numpy as np


# =========================
# CONFIG
# =========================
CSV_PATH = "trade_excursions.csv"

MIN_SAMPLES_PER_BUCKET = 50


# =========================
# BUCKET HELPERS
# =========================
def create_bucket_labels(edges):

    labels = []

    for i in range(len(edges) - 1):

        labels.append(
            f"{edges[i]:.5f} - {edges[i + 1]:.5f}"
        )

    return labels

# =========================
# ANALYZE BUCKETS
# =========================
def analyze_buckets(
    df,
    column_name,
    bucket_edges
):

    print("\n====================================")
    print(f"📊 ANALYZING: {column_name}")
    print("====================================")

    labels = create_bucket_labels(bucket_edges)

    bucket_col = f"{column_name}_Bucket"

    df[bucket_col] = pd.cut(
        df[column_name],
        bins=bucket_edges,
        labels=labels,
        include_lowest=True
    )

    rows = []

    grouped = df.groupby(bucket_col)

    for bucket_name, subset in grouped:

        if len(subset) < MIN_SAMPLES_PER_BUCKET:
            continue

        avg_mfe = subset["MFE_Pips"].mean()
        median_mfe = subset["MFE_Pips"].median()

        avg_mae = subset["MAE_Pips"].mean()
        median_mae = subset["MAE_Pips"].median()

        avg_final = subset["FinalMove_Pips"].mean()

        positive_final_pct = (
            subset["FinalMove_Pips"] > 0
        ).mean()

        huge_move_pct = (
            subset["MFE_Pips"] >= 1000
        ).mean()

        mfe_mae_ratio = (
            avg_mfe / avg_mae
            if avg_mae != 0 else None
        )

        p90_mfe = subset["MFE_Pips"].quantile(0.90)
        p95_mfe = subset["MFE_Pips"].quantile(0.95)

        p90_mae = subset["MAE_Pips"].quantile(0.90)
        p95_mae = subset["MAE_Pips"].quantile(0.95)

        rows.append({

            "Bucket": bucket_name,

            "Samples": len(subset),

            "AvgMFE": avg_mfe,
            "MedianMFE": median_mfe,

            "AvgMAE": avg_mae,
            "MedianMAE": median_mae,

            "MFE_MAE_Ratio": mfe_mae_ratio,

            "AvgFinalMove": avg_final,

            "PositiveFinalPct": positive_final_pct,

            "HugeMovePct": huge_move_pct,

            "P90_MFE": p90_mfe,
            "P95_MFE": p95_mfe,

            "P90_MAE": p90_mae,
            "P95_MAE": p95_mae
        })

    result_df = pd.DataFrame(rows)

    if result_df.empty:

        print("No valid buckets.")

        return result_df

    result_df = result_df.sort_values(
        "MFE_MAE_Ratio",
        ascending=False
    )

    print(result_df)

    return result_df


# =========================
# MAIN
# =========================
def run():

    print("\n====================================")
    print("🚀 TRADE DISTRIBUTION ANALYZER")
    print("====================================")

    # =========================
    # LOAD CSV
    # =========================
    df = pd.read_csv(CSV_PATH)

    print(f"\nLoaded rows: {len(df)}")

    # =========================
    # AGREEMENT ANALYSIS
    # =========================
    agreement_edges = [
        0.50,
        0.60,
        0.70,
        0.80,
        0.90,
        1.01
    ]

    agreement_results = analyze_buckets(
        df,
        "DirectionAgreement",
        agreement_edges
    )

    # =========================
    # DISTANCE ANALYSIS
    # =========================
    distance_edges = [
        0.00,
        0.013,
        0.014,
        0.015,
        0.017,
        0.020,
        1.00
    ]

    distance_results = analyze_buckets(
        df,
        "AvgDistance",
        distance_edges
    )

    # =========================
    # H1 SLOPE ANALYSIS
    # =========================
    slope_edges = [
        0.0,
        0.5,
        1.0,
        2.0,
        3.0,
        5.0,
        100.0
    ]

    slope_results = analyze_buckets(
        df,
        "Abs_H1_EMA20_Slope",
        slope_edges
    )

    # =========================
    # SAVE RESULTS
    # =========================
    agreement_results.to_csv(
        "agreement_distribution_analysis.csv",
        index=False
    )

    distance_results.to_csv(
        "distance_distribution_analysis.csv",
        index=False
    )

    slope_results.to_csv(
        "slope_distribution_analysis.csv",
        index=False
    )

    print("\n====================================")
    print("✅ DISTRIBUTION ANALYSIS COMPLETE")
    print("====================================")

    print("\n💾 Files saved:")
    print("- agreement_distribution_analysis.csv")
    print("- distance_distribution_analysis.csv")
    print("- slope_distribution_analysis.csv")


# =========================
# ENTRY
# =========================
if __name__ == "__main__":

    run()