import pandas as pd
import numpy as np

from ClusterEdgeFinder import build_signals
from CurrentLoader import load_history as load_current_data
from FutureExtractor import TLLEN


# =========================
# CONFIG
# =========================
TP_PIPS = 45.0
SL_PIPS = 30.0

PIP_VALUE = 0.01

MAX_BARS = 12

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
# SIMULATE ONE TRADE (OHLC)
# =========================
def simulate_trade(
    entry_idx,
    direction,
    highs,
    lows,
    closes
):

    if entry_idx + MAX_BARS >= len(closes):
        return None

    entry_price = closes[entry_idx]

    tp_distance = TP_PIPS * PIP_VALUE
    sl_distance = SL_PIPS * PIP_VALUE

    # =========================
    # LONG
    # =========================
    if direction == 1:

        tp_price = entry_price + tp_distance
        sl_price = entry_price - sl_distance

    # =========================
    # SHORT
    # =========================
    else:

        tp_price = entry_price - tp_distance
        sl_price = entry_price + sl_distance

    # =========================
    # FORWARD WALK
    # =========================
    for i in range(1, MAX_BARS + 1):

        high = highs[entry_idx + i]
        low = lows[entry_idx + i]

        # =========================
        # LONG
        # =========================
        if direction == 1:

            # SL FIRST (conservative)
            if low <= sl_price:

                return {
                    "Result": "SL",
                    "PnL": -SL_PIPS,
                    "BarsHeld": i
                }

            if high >= tp_price:

                return {
                    "Result": "TP",
                    "PnL": TP_PIPS,
                    "BarsHeld": i
                }

        # =========================
        # SHORT
        # =========================
        else:

            # SL FIRST (conservative)
            if high >= sl_price:

                return {
                    "Result": "SL",
                    "PnL": -SL_PIPS,
                    "BarsHeld": i
                }

            if low <= tp_price:

                return {
                    "Result": "TP",
                    "PnL": TP_PIPS,
                    "BarsHeld": i
                }

    # =========================
    # TIME EXIT
    # =========================
    final_price = closes[entry_idx + MAX_BARS]

    if direction == 1:

        pnl = (
            (final_price - entry_price)
            / PIP_VALUE
        )

    else:

        pnl = (
            (entry_price - final_price)
            / PIP_VALUE
        )

    return {
        "Result": "TIME",
        "PnL": pnl,
        "BarsHeld": MAX_BARS
    }


# =========================
# MAIN
# =========================
def run():

    print("\n==============================")
    print("🚀 TRADE SIMULATION ANALYZER")
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

    ]

    print(f"\nSignals after filters: {len(signals_df)}")

    # =========================
    # LOAD CURRENT DATA
    # =========================
    candles_curr, _ = load_current_data()

    tp_to_idx, tp_list = build_index_map(candles_curr)

    closes = np.array([
        candles_curr[tp]["close"]
        for tp in tp_list
    ])

    highs = np.array([
        candles_curr[tp]["high"]
        for tp in tp_list
    ])

    lows = np.array([
        candles_curr[tp]["low"]
        for tp in tp_list
    ])

    # =========================
    # RUN SIMULATION
    # =========================
    trades = []

    for _, row in signals_df.iterrows():

        curr_tp = row["CurrentTP"]

        if curr_tp not in tp_to_idx:
            continue

        entry_idx = tp_to_idx[curr_tp] + TLLEN

        direction = row["SignalDirection"]

        result = simulate_trade(
            entry_idx,
            direction,
            highs,
            lows,
            closes
        )

        if result is None:
            continue

        trades.append({

            "CurrentTP": curr_tp,

            "Direction": direction,

            "Agreement": row["DirectionAgreement"],

            "AvgDistance": row["AvgDistance"],

            "Result": result["Result"],

            "PnL": result["PnL"],

            "BarsHeld": result["BarsHeld"]
        })

        if len(trades) % 500 == 0:
            print(f"Processed trades: {len(trades)}")

    # =========================
    # RESULTS
    # =========================
    trades_df = pd.DataFrame(trades)

    print("\n==============================")
    print("✅ TRADE SIMULATION COMPLETE")
    print("==============================")

    print(trades_df.head())

    # =========================
    # STATS
    # =========================
    total_trades = len(trades_df)

    if total_trades == 0:

        print("\n❌ No trades")

        return

    winrate = (trades_df["PnL"] > 0).mean()

    avg_pnl = trades_df["PnL"].mean()

    total_pnl = trades_df["PnL"].sum()

    tp_hits = (trades_df["Result"] == "TP").mean()

    sl_hits = (trades_df["Result"] == "SL").mean()

    time_exits = (trades_df["Result"] == "TIME").mean()

    print(f"\nTotal Trades: {total_trades}")

    print(f"WinRate: {winrate:.4f}")

    print(f"Avg PnL: {avg_pnl:.2f} pips")

    print(f"Total PnL: {total_pnl:.2f} pips")

    print(f"TP Hits: {tp_hits:.4f}")

    print(f"SL Hits: {sl_hits:.4f}")

    print(f"Time Exits: {time_exits:.4f}")

    print(f"Average Bars Held: {trades_df['BarsHeld'].mean():.2f}")

    # =========================
    # SAVE
    # =========================
    trades_df.to_csv(
        "trade_simulation_results.csv",
        index=False
    )

    print("\n💾 Saved to trade_simulation_results.csv")

    return trades_df


# =========================
# ENTRY
# =========================
if __name__ == "__main__":

    run()