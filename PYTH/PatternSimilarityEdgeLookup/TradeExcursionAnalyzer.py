import pandas as pd
import numpy as np

from ClusterEdgeFinder import build_signals
from CurrentLoader import load_history as load_current_data
from FutureExtractor import TLLEN


# =========================
# CONFIG
# =========================
LOOKAHEAD_BARS = 12

PIP_VALUE = 0.01

MIN_AGREEMENT = 0.60
MAX_DISTANCE = 0.014
MIN_SLOPE = 0.0


# =========================
# BUILD INDEX MAP
# =========================
def build_index_map(candles):
    timepoints = sorted(candles.keys())

    return (
        {tp: i for i, tp in enumerate(timepoints)},
        timepoints
    )


# =========================
# EXCURSION CALCULATION
# =========================
def calculate_excursion(entry_idx, direction, highs, lows, closes):

    if entry_idx + LOOKAHEAD_BARS >= len(closes):
        return None

    entry_price = closes[entry_idx]

    future_highs = highs[entry_idx + 1: entry_idx + LOOKAHEAD_BARS + 1]
    future_lows = lows[entry_idx + 1: entry_idx + LOOKAHEAD_BARS + 1]
    future_closes = closes[entry_idx + 1: entry_idx + LOOKAHEAD_BARS + 1]

    if len(future_highs) == 0:
        return None

    # =========================
    # LONG
    # =========================
    if direction == 1:

        max_favorable_price = np.max(future_highs)
        max_adverse_price = np.min(future_lows)

        mfe_price = max_favorable_price - entry_price
        mae_price = entry_price - max_adverse_price

        bars_to_mfe = int(np.argmax(future_highs)) + 1
        bars_to_mae = int(np.argmin(future_lows)) + 1

        final_move_price = future_closes[-1] - entry_price

    # =========================
    # SHORT
    # =========================
    else:

        max_favorable_price = np.min(future_lows)
        max_adverse_price = np.max(future_highs)

        mfe_price = entry_price - max_favorable_price
        mae_price = max_adverse_price - entry_price

        bars_to_mfe = int(np.argmin(future_lows)) + 1
        bars_to_mae = int(np.argmax(future_highs)) + 1

        final_move_price = entry_price - future_closes[-1]

    mfe_pips = mfe_price / PIP_VALUE
    mae_pips = mae_price / PIP_VALUE
    final_move_pips = final_move_price / PIP_VALUE

    return {
        "EntryPrice": entry_price,

        "MFE_Price": mfe_price,
        "MAE_Price": mae_price,

        "MFE_Pips": mfe_pips,
        "MAE_Pips": mae_pips,

        "BarsToMFE": bars_to_mfe,
        "BarsToMAE": bars_to_mae,

        "FinalMove_Price": final_move_price,
        "FinalMove_Pips": final_move_pips,

        "MFE_MAE_Ratio": mfe_pips / mae_pips if mae_pips != 0 else None
    }


# =========================
# MAIN
# =========================
def run():

    print("\n==============================")
    print("🚀 TRADE EXCURSION ANALYZER")
    print("==============================")

    # =========================
    # LOAD SIGNALS
    # =========================
    signals_df = build_signals()

    # =========================
    # FILTER SIGNALS
    # =========================
    signals_df = signals_df[
        (signals_df["DirectionAgreement"] >= MIN_AGREEMENT) &
        (signals_df["AvgDistance"] <= MAX_DISTANCE) &
        (signals_df["Abs_H1_EMA20_Slope"] >= MIN_SLOPE)
    ].copy()

    print(f"\nSignals after filters: {len(signals_df)}")

    if signals_df.empty:
        print("No signals after filters.")
        return pd.DataFrame()

    # =========================
    # LOAD CURRENT DATA
    # =========================
    candles_curr, _ = load_current_data()

    tp_to_idx, tp_list = build_index_map(candles_curr)

    closes = np.array([
        candles_curr[tp]["close"]
        for tp in tp_list
    ], dtype=np.float64)

    highs = np.array([
        candles_curr[tp]["high"]
        for tp in tp_list
    ], dtype=np.float64)

    lows = np.array([
        candles_curr[tp]["low"]
        for tp in tp_list
    ], dtype=np.float64)

    # =========================
    # RUN EXCURSION ANALYSIS
    # =========================
    rows = []

    for _, signal in signals_df.iterrows():

        curr_tp = int(signal["CurrentTP"])

        if curr_tp not in tp_to_idx:
            continue

        entry_idx = tp_to_idx[curr_tp] + TLLEN

        direction = int(signal["SignalDirection"])

        excursion = calculate_excursion(
            entry_idx,
            direction,
            highs,
            lows,
            closes
        )

        if excursion is None:
            continue

        rows.append({
            "CurrentTP": curr_tp,
            "SignalDirection": direction,

            "DirectionAgreement": signal["DirectionAgreement"],
            "AvgDistance": signal["AvgDistance"],
            "MinDistance": signal["MinDistance"],
            "BestRank": signal["BestRank"],
            "Samples": signal["Samples"],

            "H1_EMA20_Slope": signal["H1_EMA20_Slope"],
            "Abs_H1_EMA20_Slope": signal["Abs_H1_EMA20_Slope"],

            "PredictionCorrect": signal["PredictionCorrect"],
            "CurrReturn": signal["CurrReturn"],

            **excursion
        })

        if len(rows) % 500 == 0:
            print(f"Processed excursions: {len(rows)}")

    df = pd.DataFrame(rows)

    print("\n==============================")
    print("✅ EXCURSION ANALYSIS COMPLETE")
    print("==============================")

    if df.empty:
        print("No excursions generated.")
        return df

    print(df.head())

    # =========================
    # BASIC SUMMARY
    # =========================
    print("\n=== SUMMARY ===")
    print(f"Total Signals: {len(df)}")
    print(f"Avg MFE: {df['MFE_Pips'].mean():.2f} pips")
    print(f"Median MFE: {df['MFE_Pips'].median():.2f} pips")
    print(f"Avg MAE: {df['MAE_Pips'].mean():.2f} pips")
    print(f"Median MAE: {df['MAE_Pips'].median():.2f} pips")
    print(f"Avg Final Move: {df['FinalMove_Pips'].mean():.2f} pips")
    print(f"Avg Bars To MFE: {df['BarsToMFE'].mean():.2f}")
    print(f"Avg Bars To MAE: {df['BarsToMAE'].mean():.2f}")

    print("\n=== MFE Percentiles ===")
    print(df["MFE_Pips"].quantile([0.25, 0.5, 0.75, 0.9, 0.95]))

    print("\n=== MAE Percentiles ===")
    print(df["MAE_Pips"].quantile([0.25, 0.5, 0.75, 0.9, 0.95]))

    # =========================
    # SAVE
    # =========================
    df.to_csv("trade_excursions.csv", index=False)

    print("\n💾 Saved to trade_excursions.csv")

    return df


# =========================
# ENTRY
# =========================
if __name__ == "__main__":
    run()