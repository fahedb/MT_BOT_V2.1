import os
import json
from datetime import datetime

import pandas as pd
from sqlalchemy import create_engine, text

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

EDGE_RUN_ID = 585
EDGE_LEVEL = "Top50"  # Top50 / Top40 / Top30 / Top20 / Top10 / Top5


# =========================
# SQL CONFIG
# =========================

CONN_STR = (
    "mssql+pyodbc://localhost\\sqlexpress/Prices?"
    "driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes"
)

SAVE_STRATEGY_RUN_TO_SQL = True


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
    # "allow_buy": False,
    # "allow_sell": True,
}


# =========================
# BASIC HELPERS
# =========================

def ensure_output_dir(output_dir):
    os.makedirs(output_dir, exist_ok=True)


def now_str():
    return datetime.now().strftime("%Y%m%d_%H%M%S")


def create_sql_engine():
    return create_engine(CONN_STR)


def safe_float(value):
    if value is None:
        return None

    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def safe_int(value):
    if value is None:
        return None

    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def safe_bool(value):
    if value is None:
        return None

    return bool(value)


def get_summary_value(summary, *keys):
    for key in keys:
        if key in summary:
            return summary.get(key)

    return None


# =========================
# RUN NAME
# =========================

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


# =========================
# TRADE FLATTENING
# =========================

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


def get_trade_pnl(trade):
    for key in ["profit", "pnl", "net_profit", "trade_profit"]:
        if key in trade and trade.get(key) is not None:
            return safe_float(trade.get(key))

    return None


def get_trade_direction(trade):
    direction = trade.get("direction")

    if direction is None and isinstance(trade.get("signal_data"), dict):
        direction = trade["signal_data"].get("direction")

    if direction is None:
        return None

    return str(direction).upper()


def get_trade_close_reason(trade):
    for key in ["close_reason", "exit_reason", "reason"]:
        if key in trade and trade.get(key) is not None:
            return str(trade.get(key)).upper()

    return ""


# =========================
# OUTPUT SAVE
# =========================

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


# =========================
# VALIDATION
# =========================

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
# STRATEGY SQL STATS
# =========================

def calc_close_reason_counts(trades):
    counts = {
        "tp_count": 0,
        "sl_count": 0,
        "same_candle_tp_count": 0,
        "same_candle_sl_count": 0,
        "end_of_test_count": 0,
    }

    for trade in trades:
        reason = get_trade_close_reason(trade)

        if reason == "TP":
            counts["tp_count"] += 1
        elif reason == "SL":
            counts["sl_count"] += 1
        elif reason == "TP_SAME_CANDLE":
            counts["same_candle_tp_count"] += 1
        elif reason == "SL_SAME_CANDLE":
            counts["same_candle_sl_count"] += 1
        elif reason == "END_OF_TEST":
            counts["end_of_test_count"] += 1

    return counts

def is_trade_win(trade):
    reason = get_trade_close_reason(trade)

    if reason in ["TP", "TP_SAME_CANDLE"]:
        return True

    if reason in ["SL", "SL_SAME_CANDLE"]:
        return False

    pnl = get_trade_pnl(trade)

    if pnl is not None:
        return pnl > 0

    return None


def calc_direction_stats(trades):
    buy_trades = 0
    sell_trades = 0
    buy_wins = 0
    sell_wins = 0

    for trade in trades:
        direction = get_trade_direction(trade)
        is_win = is_trade_win(trade)

        if direction == "BUY":
            buy_trades += 1

            if is_win is True:
                buy_wins += 1

        elif direction == "SELL":
            sell_trades += 1

            if is_win is True:
                sell_wins += 1

    buy_win_rate = buy_wins / buy_trades if buy_trades > 0 else None
    sell_win_rate = sell_wins / sell_trades if sell_trades > 0 else None

    return {
        "buy_trades": buy_trades,
        "sell_trades": sell_trades,
        "buy_wins": buy_wins,
        "sell_wins": sell_wins,
        "buy_win_rate": buy_win_rate,
        "sell_win_rate": sell_win_rate,
    }

    
def calc_trade_pnl_stats(trades):
    pnls = []

    for trade in trades:
        pnl = get_trade_pnl(trade)

        if pnl is not None:
            pnls.append(pnl)

    if len(pnls) == 0:
        return {
            "gross_profit": None,
            "gross_loss": None,
            "profit_factor": None,
            "avg_profit_per_trade": None,
            "avg_win": None,
            "avg_loss": None,
            "best_trade": None,
            "worst_trade": None,
        }

    wins = [x for x in pnls if x > 0]
    losses = [x for x in pnls if x < 0]

    gross_profit = sum(wins)
    gross_loss = abs(sum(losses))

    profit_factor = None
    if gross_loss > 0:
        profit_factor = gross_profit / gross_loss

    return {
        "gross_profit": gross_profit,
        "gross_loss": gross_loss,
        "profit_factor": profit_factor,
        "avg_profit_per_trade": sum(pnls) / len(pnls),
        "avg_win": sum(wins) / len(wins) if len(wins) > 0 else None,
        "avg_loss": sum(losses) / len(losses) if len(losses) > 0 else None,
        "best_trade": max(pnls),
        "worst_trade": min(pnls),
    }


def calc_win_loss_counts_from_trades(trades):
    wins = 0
    losses = 0

    for trade in trades:
        pnl = get_trade_pnl(trade)

        if pnl is None:
            continue

        if pnl > 0:
            wins += 1
        elif pnl < 0:
            losses += 1

    return wins, losses


# =========================
# SAVE STRATEGY RUN TO SQL
# =========================

def save_strategy_run_to_sql(summary, trades, output_paths):
    """
    Saves one Strategy Tester run summary to dbo.StrategyTestRuns.
    Each row = one RunStrategyTest execution.
    """

    simulator_config = summary.get("simulator_config", {}) or {}
    signal_config = summary.get("signal_config", {}) or {}
    strategy_config = summary.get("strategy_config", {}) or {}
    run_metadata = summary.get("run_metadata", {}) or {}

    initial_balance = safe_float(
        simulator_config.get("initial_balance")
        or get_summary_value(summary, "initial_balance", "start_balance", "starting_balance")
    )

    final_balance = safe_float(
        get_summary_value(
            summary,
            "final_balance",
            "balance",
            "ending_balance",
            "FinalBalance"
        )
    )

    net_profit = safe_float(
        get_summary_value(
            summary,
            "net_profit",
            "profit",
            "total_profit",
            "NetProfit"
        )
    )

    if net_profit is None and initial_balance is not None and final_balance is not None:
        net_profit = final_balance - initial_balance

    if final_balance is None and initial_balance is not None and net_profit is not None:
        final_balance = initial_balance + net_profit

    net_profit_pct = None
    if net_profit is not None and initial_balance is not None and initial_balance != 0:
        net_profit_pct = net_profit / initial_balance

    total_trades = safe_int(
        get_summary_value(
            summary,
            "total_trades",
            "trades",
            "closed_trades_count"
        )
    )

    if total_trades is None:
        total_trades = len(trades)

    winning_trades = safe_int(
        get_summary_value(
            summary,
            "winning_trades",
            "wins",
            "win_trades"
        )
    )

    losing_trades = safe_int(
        get_summary_value(
            summary,
            "losing_trades",
            "losses",
            "loss_trades"
        )
    )

    calculated_wins, calculated_losses = calc_win_loss_counts_from_trades(trades)

    if winning_trades is None:
        winning_trades = calculated_wins

    if losing_trades is None:
        losing_trades = calculated_losses

    win_rate = safe_float(
        get_summary_value(summary, "win_rate", "winrate", "WinRate")
    )

    if win_rate is None and total_trades and total_trades > 0 and winning_trades is not None:
        win_rate = winning_trades / total_trades

    loss_rate = None
    if total_trades and total_trades > 0 and losing_trades is not None:
        loss_rate = losing_trades / total_trades
    elif win_rate is not None:
        loss_rate = 1.0 - win_rate

    close_reason_counts = calc_close_reason_counts(trades)
    direction_stats = calc_direction_stats(trades)
    pnl_stats = calc_trade_pnl_stats(trades)

    profit_factor = safe_float(
        get_summary_value(summary, "profit_factor", "ProfitFactor")
    )

    if profit_factor is None:
        profit_factor = pnl_stats["profit_factor"]

    max_drawdown = safe_float(
        get_summary_value(summary, "max_drawdown", "MaxDrawdown")
    )

    max_drawdown_pct = safe_float(
        get_summary_value(summary, "max_drawdown_pct", "MaxDrawdownPct")
    )

    row_data = {
        # Link to Edge / Match
        "EdgeRunID": safe_int(summary.get("edge_run_id")),
        "MatchRunID": safe_int(summary.get("match_run_id")),
        "EdgeLevel": summary.get("edge_level"),
        "EdgeThreshold": safe_float(summary.get("edge_threshold")),
        "EdgeConfigName": summary.get("edge_config_name"),
        "MatchesFile": summary.get("matches_file"),

        # Strategy / Signal metadata
        "SignalDirectionMethod": signal_config.get("signal_direction_method"),
        "FutureDirectionMethod": signal_config.get("future_direction_method"),

        "FirstHitPips": safe_int(signal_config.get("first_hit_pips")),
        "FirstHitFutureWindow": safe_int(signal_config.get("first_hit_future_window")),

        "MinAbsPredReturn": safe_float(signal_config.get("min_abs_pred_return")),
        "MinDirectionAgreement": safe_float(signal_config.get("min_direction_agreement")),
        "MinWeightedDirectionAgreement": safe_float(signal_config.get("min_weighted_direction_agreement")),
        "MinFirstHitAgreement": safe_float(signal_config.get("min_first_hit_agreement")),
        "MinWeightedFirstHitAgreement": safe_float(signal_config.get("min_weighted_first_hit_agreement")),
        "MinValidFirstHitCount": safe_int(signal_config.get("min_valid_first_hit_count")),
        "MaxFirstHitNoiseRatio": safe_float(signal_config.get("max_first_hit_noise_ratio")),

        "EdgeDatasetRows": safe_int(run_metadata.get("edge_dataset_rows")),
        "EdgeDirectionAccuracy": safe_float(run_metadata.get("edge_direction_accuracy")),
        "EdgeLevelAccuracy": safe_float(run_metadata.get("edge_level_accuracy")),
        "EdgeLevelSamples": safe_int(run_metadata.get("edge_level_samples")),
        "EdgeLevelAvgReturn": safe_float(run_metadata.get("edge_level_avg_return")),

        # Simulator config
        "Symbol": simulator_config.get("symbol"),
        "PipSize": safe_float(simulator_config.get("pip_size")),
        "MoneyPerPip": safe_float(simulator_config.get("money_per_pip")),
        "SpreadPips": safe_float(simulator_config.get("spread_pips")),
        "SameCandlePolicy": simulator_config.get("same_candle_policy"),

        # Strategy config
        "TpPips": safe_float(strategy_config.get("tp_pips")),
        "SlPips": safe_float(strategy_config.get("sl_pips")),
        "LotSize": safe_float(strategy_config.get("lot_size")),
        "MaxOpenTrades": safe_int(strategy_config.get("max_open_trades")),
        "BlockReentrySameCandle": safe_bool(strategy_config.get("block_reentry_same_candle")),
        "MaxCandles": safe_int(strategy_config.get("max_candles")),

        # Test coverage
        "CurrentBarsProcessed": safe_int(summary.get("candles_processed")),
        "SignalsChecked": safe_int(summary.get("signals_checked")),
        "SignalsPassed": safe_int(summary.get("signals_passed")),
        "TradesOpened": safe_int(summary.get("trades_opened")),

        # Trading results
        "TotalTrades": safe_int(total_trades),
        "WinningTrades": safe_int(winning_trades),
        "LosingTrades": safe_int(losing_trades),
        "ClosedTrades": safe_int(len(trades)),
        "OpenTradesAtEnd": safe_int(
            get_summary_value(summary, "open_trades_at_end", "open_trades")
        ),

        "WinRate": safe_float(win_rate),
        "LossRate": safe_float(loss_rate),

        "InitialBalance": safe_float(initial_balance),
        "FinalBalance": safe_float(final_balance),
        "NetProfit": safe_float(net_profit),
        "NetProfitPct": safe_float(net_profit_pct),

        "GrossProfit": safe_float(
            get_summary_value(summary, "gross_profit", "GrossProfit")
        ) or safe_float(pnl_stats["gross_profit"]),

        "GrossLoss": safe_float(
            get_summary_value(summary, "gross_loss", "GrossLoss")
        ) or safe_float(pnl_stats["gross_loss"]),

        "ProfitFactor": safe_float(profit_factor),

        "MaxDrawdown": safe_float(max_drawdown),
        "MaxDrawdownPct": safe_float(max_drawdown_pct),

        "AvgProfitPerTrade": safe_float(
            get_summary_value(summary, "avg_profit_per_trade", "AvgProfitPerTrade")
        ) or safe_float(pnl_stats["avg_profit_per_trade"]),
        
        "AvgWin": safe_float(
            get_summary_value(summary, "avg_win", "AvgWin")
        ) or safe_float(pnl_stats["avg_win"]),

        "AvgLoss": safe_float(
            get_summary_value(summary, "avg_loss", "AvgLoss")
        ) or safe_float(pnl_stats["avg_loss"]),

        "BestTrade": safe_float(
            get_summary_value(summary, "best_trade", "BestTrade")
        ) or safe_float(pnl_stats["best_trade"]),

        "WorstTrade": safe_float(
            get_summary_value(summary, "worst_trade", "WorstTrade")
        ) or safe_float(pnl_stats["worst_trade"]),

        # Extra analysis
        "TpCount": safe_int(close_reason_counts["tp_count"]),
        "SlCount": safe_int(close_reason_counts["sl_count"]),
        "SameCandleTpCount": safe_int(close_reason_counts["same_candle_tp_count"]),
        "SameCandleSlCount": safe_int(close_reason_counts["same_candle_sl_count"]),
        "EndOfTestCount": safe_int(close_reason_counts["end_of_test_count"]),

        "BuyTrades": safe_int(direction_stats["buy_trades"]),
        "SellTrades": safe_int(direction_stats["sell_trades"]),
        "BuyWins": safe_int(direction_stats["buy_wins"]),
        "SellWins": safe_int(direction_stats["sell_wins"]),
        "BuyWinRate": safe_float(direction_stats["buy_win_rate"]),
        "SellWinRate": safe_float(direction_stats["sell_win_rate"]),

        # Raw configs / full data
        "SimulatorConfigJson": json.dumps(simulator_config, default=str),
        "SignalConfigJson": json.dumps(signal_config, default=str),
        "StrategyConfigJson": json.dumps(strategy_config, default=str),
        "RunMetadataJson": json.dumps(run_metadata, default=str),
        "SummaryJson": json.dumps(summary, default=str),

        # Output files
        "TradesCsvPath": output_paths.get("trades_csv_path"),
        "TradesJsonPath": output_paths.get("trades_json_path"),
        "SummaryJsonPath": output_paths.get("summary_json_path"),
    }

    insert_sql = text("""
        INSERT INTO dbo.StrategyTestRuns
        (
            EdgeRunID,
            MatchRunID,
            EdgeLevel,
            EdgeThreshold,
            EdgeConfigName,
            MatchesFile,

            SignalDirectionMethod,
            FutureDirectionMethod,
            FirstHitPips,
            FirstHitFutureWindow,

            MinAbsPredReturn,
            MinDirectionAgreement,
            MinWeightedDirectionAgreement,
            MinFirstHitAgreement,
            MinWeightedFirstHitAgreement,
            MinValidFirstHitCount,
            MaxFirstHitNoiseRatio,

            EdgeDatasetRows,
            EdgeDirectionAccuracy,
            EdgeLevelAccuracy,
            EdgeLevelSamples,
            EdgeLevelAvgReturn,

            Symbol,
            PipSize,
            MoneyPerPip,
            SpreadPips,
            SameCandlePolicy,

            TpPips,
            SlPips,
            LotSize,
            MaxOpenTrades,
            BlockReentrySameCandle,
            MaxCandles,

            CurrentBarsProcessed,
            SignalsChecked,
            SignalsPassed,
            TradesOpened,

            TotalTrades,
            WinningTrades,
            LosingTrades,
            ClosedTrades,
            OpenTradesAtEnd,

            WinRate,
            LossRate,

            InitialBalance,
            FinalBalance,
            NetProfit,
            NetProfitPct,

            GrossProfit,
            GrossLoss,
            ProfitFactor,

            MaxDrawdown,
            MaxDrawdownPct,

            AvgProfitPerTrade,
            AvgWin,
            AvgLoss,
            BestTrade,
            WorstTrade,

            TpCount,
            SlCount,
            SameCandleTpCount,
            SameCandleSlCount,
            EndOfTestCount,

            BuyTrades,
            SellTrades,
            BuyWins,
            SellWins,
            BuyWinRate,
            SellWinRate,

            SimulatorConfigJson,
            SignalConfigJson,
            StrategyConfigJson,
            RunMetadataJson,
            SummaryJson,

            TradesCsvPath,
            TradesJsonPath,
            SummaryJsonPath
        )
        OUTPUT INSERTED.StrategyRunID
        VALUES
        (
            :EdgeRunID,
            :MatchRunID,
            :EdgeLevel,
            :EdgeThreshold,
            :EdgeConfigName,
            :MatchesFile,

            :SignalDirectionMethod,
            :FutureDirectionMethod,
            :FirstHitPips,
            :FirstHitFutureWindow,

            :MinAbsPredReturn,
            :MinDirectionAgreement,
            :MinWeightedDirectionAgreement,
            :MinFirstHitAgreement,
            :MinWeightedFirstHitAgreement,
            :MinValidFirstHitCount,
            :MaxFirstHitNoiseRatio,

            :EdgeDatasetRows,
            :EdgeDirectionAccuracy,
            :EdgeLevelAccuracy,
            :EdgeLevelSamples,
            :EdgeLevelAvgReturn,

            :Symbol,
            :PipSize,
            :MoneyPerPip,
            :SpreadPips,
            :SameCandlePolicy,

            :TpPips,
            :SlPips,
            :LotSize,
            :MaxOpenTrades,
            :BlockReentrySameCandle,
            :MaxCandles,

            :CurrentBarsProcessed,
            :SignalsChecked,
            :SignalsPassed,
            :TradesOpened,

            :TotalTrades,
            :WinningTrades,
            :LosingTrades,
            :ClosedTrades,
            :OpenTradesAtEnd,

            :WinRate,
            :LossRate,

            :InitialBalance,
            :FinalBalance,
            :NetProfit,
            :NetProfitPct,

            :GrossProfit,
            :GrossLoss,
            :ProfitFactor,

            :MaxDrawdown,
            :MaxDrawdownPct,

            :AvgProfitPerTrade,
            :AvgWin,
            :AvgLoss,
            :BestTrade,
            :WorstTrade,

            :TpCount,
            :SlCount,
            :SameCandleTpCount,
            :SameCandleSlCount,
            :EndOfTestCount,

            :BuyTrades,
            :SellTrades,
            :BuyWins,
            :SellWins,
            :BuyWinRate,
            :SellWinRate,

            :SimulatorConfigJson,
            :SignalConfigJson,
            :StrategyConfigJson,
            :RunMetadataJson,
            :SummaryJson,

            :TradesCsvPath,
            :TradesJsonPath,
            :SummaryJsonPath
        )
    """)

    engine = create_sql_engine()

    with engine.begin() as conn:
        strategy_run_id = conn.execute(insert_sql, row_data).scalar()

    print("\n=== STRATEGY RUN SAVED TO SQL ===")
    print(f"StrategyRunID: {strategy_run_id}")

    return strategy_run_id


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

    strategy_run_id = None

    if SAVE_STRATEGY_RUN_TO_SQL:
        strategy_run_id = save_strategy_run_to_sql(
            summary=summary,
            trades=trades,
            output_paths=output_paths
        )

    summary["strategy_run_id"] = strategy_run_id

    return {
        "summary": summary,
        "trades": trades,
        "output_paths": output_paths,
        "strategy_run_id": strategy_run_id,
    }


if __name__ == "__main__":
    run_strategy_test()