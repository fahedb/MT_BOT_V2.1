from CandlesLoaders.CurrentLoader import load_history as load_current_data


class TradingSimulator:
    def __init__(
        self,
        initial_balance=10000.0,
        symbol="XAUUSD",
        pip_size=0.01,
        money_per_pip=1.0,
        spread_pips=0.0,
        same_candle_policy="sl_first",
        candles=None
    ):
        """
        Basic candle-by-candle trading simulator.

        Notes:
            - This simulator uses OHLC candles.
            - Trades are opened at the current candle close.
            - Open trades are checked on each next candle.
            - If SL and TP are both touched in the same candle,
              same_candle_policy decides which one wins.
        """

        self.initial_balance = float(initial_balance)
        self.balance = float(initial_balance)
        self.equity = float(initial_balance)

        self.symbol = symbol
        self.pip_size = float(pip_size)
        self.money_per_pip = float(money_per_pip)
        self.spread_pips = float(spread_pips)
        self.same_candle_policy = same_candle_policy

        if candles is None:
            self.candles, _ = load_current_data()
        else:
            self.candles = candles

        self.timepoints = sorted(self.candles.keys())
        self.current_index = -1

        self.open_trades = []
        self.closed_trades = []

        self.next_trade_id = 1

        if len(self.timepoints) == 0:
            raise ValueError("No candles loaded into TradingSimulator")

        if self.same_candle_policy not in ["sl_first", "tp_first"]:
            raise ValueError("same_candle_policy must be 'sl_first' or 'tp_first'")

        print("\n=== TRADING SIMULATOR INIT ===")
        print(f"Symbol: {self.symbol}")
        print(f"Candles: {len(self.timepoints)}")
        print(f"Initial balance: {self.initial_balance}")
        print(f"Pip size: {self.pip_size}")
        print(f"Money per pip: {self.money_per_pip}")
        print(f"Spread pips: {self.spread_pips}")
        print(f"Same candle policy: {self.same_candle_policy}")

    # =========================
    # NAVIGATION
    # =========================

    def has_next(self):
        return self.current_index + 1 < len(self.timepoints)

    def next(self):
        """
        Move simulator one candle forward and process open trades.
        """

        if not self.has_next():
            return False

        self.current_index += 1

        current_candle = self.get_current_candle()
        current_tp = self.get_current_tp()

        self._process_open_trades(
            candle=current_candle,
            candle_tp=current_tp
        )

        self._update_equity()

        return True

    def run_to_end(self):
        while self.has_next():
            self.next()

        self.close_all_open_trades(reason="END_OF_TEST")

        return self.get_summary()

    # =========================
    # CURRENT STATE
    # =========================

    def get_current_tp(self):
        if self.current_index < 0:
            return None

        return int(self.timepoints[self.current_index])

    def get_current_candle(self):
        tp = self.get_current_tp()

        if tp is None:
            return None

        return self.candles[tp]

    def get_current_price(self):
        candle = self.get_current_candle()

        if candle is None:
            return None

        return self._get_close(candle)

    def get_account_state(self):
        return {
            "initial_balance": self.initial_balance,
            "balance": self.balance,
            "equity": self.equity,
            "open_trades": len(self.open_trades),
            "closed_trades": len(self.closed_trades),
            "current_index": self.current_index,
            "current_tp": self.get_current_tp(),
        }

    def get_open_trades(self):
        return list(self.open_trades)

    def get_closed_trades(self):
        return list(self.closed_trades)

    # =========================
    # TRADE MANAGEMENT
    # =========================

    def open_trade(
        self,
        direction,
        tp_pips,
        sl_pips,
        lot_size=1.0,
        signal_data=None,
        reason=None
    ):
        """
        Opens a trade at the current candle close.

        direction:
            BUY or SELL

        tp_pips / sl_pips:
            Distance from entry in pips.
        """

        direction = direction.upper()

        if direction not in ["BUY", "SELL"]:
            raise ValueError("direction must be BUY or SELL")

        if self.current_index < 0:
            raise ValueError("Simulator has not started. Call next() before opening trades.")

        entry_tp = self.get_current_tp()
        candle = self.get_current_candle()
        raw_entry_price = self._get_close(candle)

        entry_price = self._apply_entry_spread(
            price=raw_entry_price,
            direction=direction
        )

        tp_pips = float(tp_pips)
        sl_pips = float(sl_pips)

        if direction == "BUY":
            tp_price = entry_price + (tp_pips * self.pip_size)
            sl_price = entry_price - (sl_pips * self.pip_size)

        else:
            tp_price = entry_price - (tp_pips * self.pip_size)
            sl_price = entry_price + (sl_pips * self.pip_size)

        trade = {
            "trade_id": self.next_trade_id,
            "symbol": self.symbol,
            "direction": direction,

            "entry_index": self.current_index,
            "entry_tp": int(entry_tp),
            "entry_price": float(entry_price),
            "raw_entry_price": float(raw_entry_price),

            "tp_price": float(tp_price),
            "sl_price": float(sl_price),
            "tp_pips": float(tp_pips),
            "sl_pips": float(sl_pips),

            "lot_size": float(lot_size),
            "status": "OPEN",

            "exit_index": None,
            "exit_tp": None,
            "exit_price": None,

            "profit_pips": None,
            "profit_money": None,
            "close_reason": None,

            "balance_before": float(self.balance),
            "balance_after": None,

            "signal_data": signal_data,
            "open_reason": reason,
        }

        self.next_trade_id += 1
        self.open_trades.append(trade)

        return trade

    def close_all_open_trades(self, reason="MANUAL"):
        if len(self.open_trades) == 0:
            return []

        current_price = self.get_current_price()
        current_tp = self.get_current_tp()

        if current_price is None:
            return []

        closed = []

        for trade in list(self.open_trades):
            closed_trade = self._close_trade(
                trade=trade,
                exit_price=current_price,
                exit_tp=current_tp,
                close_reason=reason
            )

            closed.append(closed_trade)

        return closed

    # =========================
    # INTERNAL TRADE PROCESSING
    # =========================

    def _process_open_trades(self, candle, candle_tp):
        if len(self.open_trades) == 0:
            return

        high = self._get_high(candle)
        low = self._get_low(candle)

        for trade in list(self.open_trades):

            direction = trade["direction"]

            if direction == "BUY":
                sl_hit = low <= trade["sl_price"]
                tp_hit = high >= trade["tp_price"]

            else:
                sl_hit = high >= trade["sl_price"]
                tp_hit = low <= trade["tp_price"]

            if not sl_hit and not tp_hit:
                continue

            exit_price, close_reason = self._resolve_hit_price(
                trade=trade,
                sl_hit=sl_hit,
                tp_hit=tp_hit
            )

            self._close_trade(
                trade=trade,
                exit_price=exit_price,
                exit_tp=candle_tp,
                close_reason=close_reason
            )

    def _resolve_hit_price(self, trade, sl_hit, tp_hit):
        if sl_hit and tp_hit:
            if self.same_candle_policy == "tp_first":
                return trade["tp_price"], "TP_SAME_CANDLE"

            return trade["sl_price"], "SL_SAME_CANDLE"

        if tp_hit:
            return trade["tp_price"], "TP"

        return trade["sl_price"], "SL"

    def _close_trade(self, trade, exit_price, exit_tp, close_reason):
        if trade not in self.open_trades:
            return trade

        exit_price = float(exit_price)

        profit_pips = self._calc_profit_pips(
            direction=trade["direction"],
            entry_price=trade["entry_price"],
            exit_price=exit_price
        )

        profit_money = (
            profit_pips
            * self.money_per_pip
            * float(trade["lot_size"])
        )

        self.balance += profit_money

        trade["status"] = "CLOSED"
        trade["exit_index"] = self.current_index
        trade["exit_tp"] = int(exit_tp) if exit_tp is not None else None
        trade["exit_price"] = float(exit_price)
        trade["profit_pips"] = float(profit_pips)
        trade["profit_money"] = float(profit_money)
        trade["close_reason"] = close_reason
        trade["balance_after"] = float(self.balance)

        self.open_trades.remove(trade)
        self.closed_trades.append(trade)

        self._update_equity()

        return trade

    # =========================
    # PROFIT / PRICES
    # =========================

    def _calc_profit_pips(self, direction, entry_price, exit_price):
        if direction == "BUY":
            return (exit_price - entry_price) / self.pip_size

        return (entry_price - exit_price) / self.pip_size

    def _apply_entry_spread(self, price, direction):
        spread_price = self.spread_pips * self.pip_size

        if direction == "BUY":
            return float(price) + spread_price

        return float(price) - spread_price

    def _update_equity(self):
        """
        For now equity = balance + unrealized PnL.
        """

        current_price = self.get_current_price()

        if current_price is None:
            self.equity = self.balance
            return

        unrealized = 0.0

        for trade in self.open_trades:
            profit_pips = self._calc_profit_pips(
                direction=trade["direction"],
                entry_price=trade["entry_price"],
                exit_price=current_price
            )

            unrealized += (
                profit_pips
                * self.money_per_pip
                * float(trade["lot_size"])
            )

        self.equity = self.balance + unrealized

    # =========================
    # SUMMARY
    # =========================

    def get_summary(self):
        closed = self.closed_trades

        total_trades = len(closed)

        if total_trades == 0:
            return {
                "initial_balance": self.initial_balance,
                "final_balance": self.balance,
                "net_profit": self.balance - self.initial_balance,
                "net_profit_percent": self._safe_div(
                    self.balance - self.initial_balance,
                    self.initial_balance
                ) * 100.0,
                "total_trades": 0,
                "wins": 0,
                "losses": 0,
                "win_rate": None,
                "profit_factor": None,
                "avg_win": None,
                "avg_loss": None,
                "max_drawdown": 0.0,
                "open_trades": len(self.open_trades),
            }

        profits = [float(t["profit_money"]) for t in closed]

        wins = [p for p in profits if p > 0]
        losses = [p for p in profits if p < 0]

        gross_profit = sum(wins)
        gross_loss = abs(sum(losses))

        win_rate = len(wins) / total_trades

        profit_factor = None
        if gross_loss > 0:
            profit_factor = gross_profit / gross_loss

        avg_win = sum(wins) / len(wins) if len(wins) else None
        avg_loss = sum(losses) / len(losses) if len(losses) else None

        max_drawdown = self._calc_max_drawdown()

        return {
            "initial_balance": self.initial_balance,
            "final_balance": self.balance,
            "net_profit": self.balance - self.initial_balance,
            "net_profit_percent": self._safe_div(
                self.balance - self.initial_balance,
                self.initial_balance
            ) * 100.0,

            "total_trades": total_trades,
            "wins": len(wins),
            "losses": len(losses),
            "win_rate": win_rate,

            "gross_profit": gross_profit,
            "gross_loss": gross_loss,
            "profit_factor": profit_factor,

            "avg_win": avg_win,
            "avg_loss": avg_loss,

            "max_drawdown": max_drawdown,
            "open_trades": len(self.open_trades),
        }

    def _calc_max_drawdown(self):
        if len(self.closed_trades) == 0:
            return 0.0

        balances = [self.initial_balance]

        for trade in self.closed_trades:
            balances.append(float(trade["balance_after"]))

        peak = balances[0]
        max_dd = 0.0

        for balance in balances:
            if balance > peak:
                peak = balance

            dd = peak - balance

            if dd > max_dd:
                max_dd = dd

        return float(max_dd)

    def _safe_div(self, numerator, denominator):
        if denominator == 0:
            return 0.0

        return numerator / denominator

    # =========================
    # CANDLE ACCESS HELPERS
    # =========================

    def _get_open(self, candle):
        return self._get_candle_value(candle, ["open", "Open", "OpenPrice"])

    def _get_high(self, candle):
        return self._get_candle_value(candle, ["high", "High", "HighPrice"])

    def _get_low(self, candle):
        return self._get_candle_value(candle, ["low", "Low", "LowPrice"])

    def _get_close(self, candle):
        return self._get_candle_value(candle, ["close", "Close", "ClosePrice"])

    def _get_candle_value(self, candle, possible_keys):
        for key in possible_keys:
            if key in candle:
                return float(candle[key])

        raise KeyError(f"Candle missing keys: {possible_keys}. Candle={candle}")


# =========================
# MANUAL TEST
# =========================
def run_manual_test():
    sim = TradingSimulator(
        initial_balance=10000,
        symbol="XAUUSD",
        pip_size=0.01,
        money_per_pip=1.0,
        spread_pips=0,
        same_candle_policy="sl_first"
    )

    # Move to first candle
    sim.next()

    trade = sim.open_trade(
        direction="BUY",
        tp_pips=200,
        sl_pips=200,
        lot_size=1.0,
        reason="manual_test"
    )

    print("\nOpened trade:")
    print(trade)

    while sim.has_next():
        sim.next()

        if len(sim.get_open_trades()) == 0:
            break

    sim.close_all_open_trades(reason="END_OF_TEST")

    print("\nClosed trades:")
    for t in sim.get_closed_trades():
        print(t)

    print("\nSummary:")
    print(sim.get_summary())


if __name__ == "__main__":
    run_manual_test()