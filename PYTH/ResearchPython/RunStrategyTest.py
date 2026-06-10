import os
import json
from datetime import datetime

import pandas as pd

from TradingSimulator.TradingSimulator import TradingSimulator
from SignalEngines.MatchSignalEngine import MatchSignalEngine

# Optional Edge-based config builder
from StrategyTools.StrategyTestConfigBuilder import build_strategy_test_payload_from_edge


# =========================
# CONFIG MODE
# =========================

# True  = build configs automatically from VectorEdgeRuns by EdgeRunID
# False = use manual configs below
USE_EDGE_RUN = True

EDGE_RUN_ID = 419
EDGE_LEVEL = "Top10"  # Top50 / Top40 / Top30 / Top20 / Top10 / Top5


# =========================
# MANUAL DEFAULT CONFIG
# Used only when USE_EDGE_RUN = False
# =========================

MATCH_RUN_ID = 17
MATCHES_FILE = f"Matches/matches_run_{MATCH_RUN_ID}.json"

OUTPUT_DIR = "StrategyResults"

SIMULATOR_CONFIG = {
    "initial_balance": 10000.0,
    "symbol": "XAUUSD",
    "pip_size": 0.01,
    "money_per_pip": 1.0,

    "spread_pips": 20.0,

    # "sl_first" / "tp_first"
    "same_candle_policy": "tp_first",
}


SIGNAL_CONFIG = {
    "min_matches_count": 5,

    "min_direction_agreement": None,
    "min_weighted_direction_agreement": None,

    "min_abs_pred_return": 0.000751,

    "signal_direction_method": "weighted_first_hit",

    "future_direction_method": "first_hit_pips",
    "first_hit_pips": 150,
    "first_hit_future_window": 20,
    "pip_size": 0.01,
    "same_candle_policy": "noise",

    "ignore_noise_in_first_hit_prediction": True,
    "min_first_hit_agreement": None,
    "min_weighted_first_hit_agreement": 0.7,
    "min_valid_first_hit_count": 7,
    "max_first_hit_noise_ratio": 0.3,

    "max_avg_distance": None,
    "max_min_distance": None,

    "allow_buy": True,
    "allow_sell": True,

    "weighting_method": "1_over_distance",
    "contrarian": False,

    "auto_export_signals": True,
    "signals_output_dir": "Signals",
    "signals_file_prefix": f"signals_match_run_{MATCH_RUN_ID}_fh150_weighted_top10",
    "export_csv": True,
    "export_json": True,
    "include_timestamp": True,
}

STRATEGY_CONFIG = {
    "tp_pips": 150,
    "sl_pips": 150,
    "lot_size": 1.0,

    "max_open_trades": 1,
    "block_reentry_same_candle": True,

    "max_candles": None,

    "save_trades_csv": True,
    "save_trades_json": True,
    "save_summary_json": True,
}


# =========================
# EDGE OVERRIDES
# Used only when USE_EDGE_RUN = True
# =========================

EDGE_SIMULATOR_OVERRIDES = {
    "initial_balance": 10000.0,
    "symbol": "XAUUSD",
    "pip_size": 0.01,
    "money_per_pip": 1.0,

    # Change this when testing costs
    "spread_pips": 20.0,

    # Test both "sl_first" and "tp_first"
    "same_candle_policy": "sl_first",
}

EDGE_STRATEGY_OVERRIDES = {
    # None = use first_hit_pips from the Edge config
    "tp_pips": None,
    "sl_pips": None,

    "lot_size": 1.0,
    "max_open_trades": 5,
    "block_reentry_same_candle": True,

    "max_candles": None,

    "save_trades_csv": True,
    "save_trades_json": True,
    "save_summary_json": True,
}

EDGE_SIGNAL_OVERRIDES = {
    # Optional manual overrides.
    # Example:
    # "min_abs_pred_return": 0.001,
    # "allow_buy": True,
    # "allow_sell": True,
}


# =========================
# HELPERS
# =========================

def ensure_output_dir(output_dir):
    os.makedirs(output_dir, exist_ok=True)


def now_str():
    return datetime.now().strftime("%Y%m%d_%H%M%S")


def make_run_name(
    match_run_id,
    strategy_config,
    signal_config,
    simulator_config=None,
    run_metadata=None
):
    run_metadata = run_metadata or {}
    simulator_config = simulator_config or {}

    parts = [
        f"strategy_match_run_{match_run_id}",
        f"tp_{strategy_config.get('tp_pips')}",
        f"sl_{strategy_config.get('sl_pips')}",
    ]

    edge_run_id = run_metadata.get("edge_run_id")
    edge_level = run_metadata.get("edge_level")

    if edge_run_id is not None:
        parts.append(f"edge_{edge_run_id}")

    if edge_level is not None:
        parts.append(str(edge_level).lower())

    spread_pips = simulator_config.get("spread_pips")
    if spread_pips is not None:
        parts.append(f"spread_{spread_pips}")

    same_candle_policy = simulator_config.get("same_candle_policy")
    if same_candle_policy is not None:
        parts.append(str(same_candle_policy))

    signal_direction_method = signal_config.get("signal_direction_method")
    if signal_direction_method is not None:
        parts.append(str(signal_direction_method))

    parts.append(now_str())

    return "_".join(parts)


def flatten_trade_for_csv(trade):
    row = dict(trade)

    signal_data = row.get("signal_data")

    if isinstance(signal_data, dict):
        row["signal_should_trade"] = signal_data.get("should_trade")
        row["signal_direction"] = signal_data.get("direction")
        row["signal_pred_return"] = signal_data.get("pred_return")
        row["signal_abs_pred_return"] = signal_data.get("abs_pred_return")

        row["signal_hist_direction_agreement"] = signal_data.get("hist_direction_agreement")
        row["signal_weighted_hist_direction_agreement"] = signal_data.get("weighted_hist_direction_agreement")

        row["signal_hist_first_hit_agreement"] = signal_data.get("hist_first_hit_agreement")
        row["signal_weighted_hist_first_hit_agreement"] = signal_data.get("weighted_hist_first_hit_agreement")
        row["signal_hist_first_hit_noise_ratio"] = signal_data.get("hist_first_hit_noise_ratio")
        row["signal_hist_first_hit_valid_count"] = signal_data.get("hist_first_hit_valid_count")

        row["signal_pred_first_hit_direction"] = signal_data.get("pred_first_hit_direction")
        row["signal_weighted_pred_first_hit_direction"] = signal_data.get("weighted_pred_first_hit_direction")

        row["signal_matches_count"] = signal_data.get("matches_count")
        row["signal_avg_distance"] = signal_data.get("avg_distance")
        row["signal_min_distance"] = signal_data.get("min_distance")
        row["signal_reason"] = signal_data.get("reason")

        row["signal_data_json"] = json.dumps(signal_data, default=str)

    else:
        row["signal_data_json"] = None

    if "signal_data" in row:
        del row["signal_data"]

    return row


def save_outputs(
    run_name,
    trades,
    summary,
    strategy_config,
    output_dir
):
    ensure_output_dir(output_dir)

    trades_csv_path = None
    trades_json_path = None
    summary_json_path = None

    trades_for_df = [
        flatten_trade_for_csv(trade)
        for trade in trades
    ]

    df_trades = pd.DataFrame(trades_for_df)

    if strategy_config.get("save_trades_csv", True):
        trades_csv_path = os.path.join(output_dir, f"{run_name}_trades.csv")
        df_trades.to_csv(trades_csv_path, index=False)

    if strategy_config.get("save_trades_json", True):
        trades_json_path = os.path.join(output_dir, f"{run_name}_trades.json")

        with open(trades_json_path, "w") as f:
            json.dump(trades, f, indent=2, default=str)

    if strategy_config.get("save_summary_json", True):
        summary_json_path = os.path.join(output_dir, f"{run_name}_summary.json")

        with open(summary_json_path, "w") as f:
            json.dump(summary, f, indent=2, default=str)

    print("\n=== STRATEGY OUTPUTS SAVED ===")

    if trades_csv_path:
        print(f"Trades CSV: {trades_csv_path}")

    if trades_json_path:
        print(f"Trades JSON: {trades_json_path}")

    if summary_json_path:
        print(f"Summary JSON: {summary_json_path}")

    return {
        "trades_csv_path": trades_csv_path,
        "trades_json_path": trades_json_path,
        "summary_json_path": summary_json_path,
    }


def print_summary(summary):
    print("\n" + "=" * 80)
    print("STRATEGY SUMMARY")
    print("=" * 80)

    for key, value in summary.items():
        print(f"{key}: {value}")

    print("=" * 80)


def validate_config(
    match_run_id,
    matches_file,
    simulator_config,
    signal_config,
    strategy_config
):
    if match_run_id is None:
        raise ValueError("match_run_id is required")

    if not matches_file:
        raise ValueError("matches_file is required")

    required_sim_keys = [
        "initial_balance",
        "symbol",
        "pip_size",
        "money_per_pip",
        "spread_pips",
        "same_candle_policy",
    ]

    for key in required_sim_keys:
        if key not in simulator_config:
            raise ValueError(f"Missing simulator_config key: {key}")

    required_strategy_keys = [
        "tp_pips",
        "sl_pips",
        "lot_size",
        "max_open_trades",
        "block_reentry_same_candle",
    ]

    for key in required_strategy_keys:
        if key not in strategy_config:
            raise ValueError(f"Missing strategy_config key: {key}")

    if strategy_config["tp_pips"] is None:
        raise ValueError("strategy_config['tp_pips'] cannot be None")

    if strategy_config["sl_pips"] is None:
        raise ValueError("strategy_config['sl_pips'] cannot be None")

    if int(strategy_config["max_open_trades"]) <= 0:
        raise ValueError("strategy_config['max_open_trades'] must be > 0")

    if "signal_direction_method" not in signal_config:
        raise ValueError("Missing signal_config key: signal_direction_method")


# =========================
# CONFIG RESOLVER
# =========================

def resolve_runtime_config():
    """
    Resolves the actual configs used by the strategy runner.

    If USE_EDGE_RUN=True:
        load EdgeRun from DB and build all configs from it.

    If USE_EDGE_RUN=False:
        use manual configs from this file.
    """

    if not USE_EDGE_RUN:
        return {
            "match_run_id": MATCH_RUN_ID,
            "matches_file": MATCHES_FILE,
            "simulator_config": SIMULATOR_CONFIG,
            "signal_config": SIGNAL_CONFIG,
            "strategy_config": STRATEGY_CONFIG,
            "run_metadata": {},
            "output_dir": OUTPUT_DIR,
        }

    payload = build_strategy_test_payload_from_edge(
        edge_run_id=EDGE_RUN_ID,
        edge_level=EDGE_LEVEL,
        simulator_overrides=EDGE_SIMULATOR_OVERRIDES,
        signal_overrides=EDGE_SIGNAL_OVERRIDES,
        strategy_overrides=EDGE_STRATEGY_OVERRIDES,
    )

    return {
        "match_run_id": payload["match_run_id"],
        "matches_file": payload["matches_file"],
        "simulator_config": payload["simulator_config"],
        "signal_config": payload["signal_config"],
        "strategy_config": payload["strategy_config"],
        "run_metadata": payload["run_metadata"],
        "output_dir": OUTPUT_DIR,
    }


# =========================
# STRATEGY RUNNER
# =========================

def run_strategy_test(
    match_run_id=None,
    matches_file=None,
    simulator_config=None,
    signal_config=None,
    strategy_config=None,
    output_dir=None,
    run_metadata=None,
):
    """
    Run strategy test.

    Can be used in two ways:

    1. Direct run:
        python RunStrategyTest.py

    2. External call:
        run_strategy_test(
            match_run_id=...,
            matches_file=...,
            simulator_config=...,
            signal_config=...,
            strategy_config=...,
            run_metadata=...
        )
    """

    if (
        match_run_id is None and
        matches_file is None and
        simulator_config is None and
        signal_config is None and
        strategy_config is None
    ):
        resolved = resolve_runtime_config()

        match_run_id = resolved["match_run_id"]
        matches_file = resolved["matches_file"]
        simulator_config = resolved["simulator_config"]
        signal_config = resolved["signal_config"]
        strategy_config = resolved["strategy_config"]
        output_dir = resolved["output_dir"]
        run_metadata = resolved["run_metadata"]

    else:
        match_run_id = match_run_id if match_run_id is not None else MATCH_RUN_ID
        matches_file = matches_file if matches_file is not None else MATCHES_FILE

        simulator_config = simulator_config if simulator_config is not None else SIMULATOR_CONFIG
        signal_config = signal_config if signal_config is not None else SIGNAL_CONFIG
        strategy_config = strategy_config if strategy_config is not None else STRATEGY_CONFIG
        output_dir = output_dir if output_dir is not None else OUTPUT_DIR
        run_metadata = run_metadata or {}

    validate_config(
        match_run_id=match_run_id,
        matches_file=matches_file,
        simulator_config=simulator_config,
        signal_config=signal_config,
        strategy_config=strategy_config
    )

    print("\n=== RUN STRATEGY TEST ===")
    print(f"UseEdgeRun: {USE_EDGE_RUN}")
    print(f"MatchRunID: {match_run_id}")
    print(f"MatchesFile: {matches_file}")
    print(f"SimulatorConfig: {simulator_config}")
    print(f"SignalConfig: {signal_config}")
    print(f"StrategyConfig: {strategy_config}")

    if run_metadata:
        print(f"RunMetadata: {run_metadata}")

    simulator = TradingSimulator(
        initial_balance=simulator_config["initial_balance"],
        symbol=simulator_config["symbol"],
        pip_size=simulator_config["pip_size"],
        money_per_pip=simulator_config["money_per_pip"],
        spread_pips=simulator_config["spread_pips"],
        same_candle_policy=simulator_config["same_candle_policy"],
    )

    signal_engine = MatchSignalEngine(
        matches_file=matches_file,
        signal_config=signal_config
    )

    if signal_config.get("auto_export_signals", False):
        signal_engine.run_and_export_signals()

    candles_processed = 0
    signals_checked = 0
    signals_passed = 0
    trades_opened = 0

    last_closed_trade_tp = None

    max_candles = strategy_config.get("max_candles")

    while simulator.has_next():

        simulator.next()
        candles_processed += 1

        current_tp = simulator.get_current_tp()

        if current_tp is None:
            continue

        if max_candles is not None and candles_processed >= int(max_candles):
            print(f"\nReached max_candles={max_candles}")
            break

        closed_trades = simulator.get_closed_trades()

        if len(closed_trades) > 0:
            last_closed_trade_tp = closed_trades[-1].get("exit_tp")

        if not signal_engine.has_signal_for_tp(current_tp):
            continue

        signals_checked += 1

        signal = signal_engine.get_signal(current_tp)

        if not signal["should_trade"]:
            continue

        signals_passed += 1

        if len(simulator.get_open_trades()) >= int(strategy_config["max_open_trades"]):
            continue

        if strategy_config.get("block_reentry_same_candle", True):
            if last_closed_trade_tp == current_tp:
                continue

        simulator.open_trade(
            direction=signal["direction"],
            tp_pips=strategy_config["tp_pips"],
            sl_pips=strategy_config["sl_pips"],
            lot_size=strategy_config["lot_size"],
            signal_data=signal,
            reason=f"MatchRunID={match_run_id}"
        )

        trades_opened += 1

        if trades_opened % 50 == 0:
            print(
                f"Trades opened: {trades_opened} | "
                f"CurrentTP: {current_tp} | "
                f"Balance: {simulator.balance}"
            )

    simulator.close_all_open_trades(reason="END_OF_TEST")

    summary = simulator.get_summary()

    summary["match_run_id"] = match_run_id
    summary["matches_file"] = matches_file

    summary["candles_processed"] = candles_processed
    summary["signals_checked"] = signals_checked
    summary["signals_passed"] = signals_passed
    summary["trades_opened"] = trades_opened

    summary["simulator_config"] = simulator_config
    summary["signal_config"] = signal_config
    summary["strategy_config"] = strategy_config

    summary["run_metadata"] = run_metadata

    summary["edge_run_id"] = run_metadata.get("edge_run_id")
    summary["edge_level"] = run_metadata.get("edge_level")
    summary["edge_threshold"] = run_metadata.get("edge_threshold")
    summary["edge_config_name"] = run_metadata.get("edge_config_name")
    summary["edge_first_hit_pips"] = run_metadata.get("edge_first_hit_pips")
    summary["edge_first_hit_future_window"] = run_metadata.get("edge_first_hit_future_window")

    trades = simulator.get_closed_trades()

    print_summary(summary)

    run_name = make_run_name(
        match_run_id=match_run_id,
        strategy_config=strategy_config,
        signal_config=signal_config,
        simulator_config=simulator_config,
        run_metadata=run_metadata
    )

    output_paths = save_outputs(
        run_name=run_name,
        trades=trades,
        summary=summary,
        strategy_config=strategy_config,
        output_dir=output_dir
    )

    return {
        "summary": summary,
        "trades": trades,
        "output_paths": output_paths,
    }


if __name__ == "__main__":
    run_strategy_test()