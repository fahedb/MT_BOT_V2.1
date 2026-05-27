import pandas as pd


def apply_rule(df, rule):

    if rule.operator == ">":
        return df[rule.feature] > rule.threshold

    elif rule.operator == "<":
        return df[rule.feature] < rule.threshold

    else:
        raise ValueError(f"Unknown operator: {rule.operator}")


def evaluate_on_test_for_db(df_test, top_rules, train_results_df, stat_type):

    rows = []

    for i, rule in enumerate(top_rules):

        mask = apply_rule(df_test, rule)
        subset = df_test[mask]

        total = len(subset)

        if total == 0:
            test_wr = 0.0
        else:
            test_wr = float(subset["IsWin"].mean())

        test_edge = test_wr - 0.5

        # TRAIN DATA
        train_row = train_results_df.iloc[i]

        train_wr = float(train_row["WinRate"])
        train_edge = float(train_row["Edge"])
        train_trades = int(train_row["Trades"])

        # METRICS
        delta = test_wr - train_wr
        stability = abs(delta)
        score = test_edge * total

        rows.append({
            "RuleDescription": str(rule),
            "StatType": stat_type,

            "TrainTrades": train_trades,
            "TrainWinRate": train_wr,
            "TrainEdge": train_edge,

            "TestTrades": total,
            "TestWinRate": test_wr,
            "TestEdge": test_edge,

            "Delta": delta,
            "Stability": stability,
            "Score": score
        })

    return rows