import pandas as pd


# =========================
# APPLY RULE
# =========================
def apply_rule(df, rule):

    if rule.operator == ">":
        return df[rule.feature] > rule.threshold

    elif rule.operator == "<":
        return df[rule.feature] < rule.threshold

    else:
        raise ValueError(f"Unknown operator: {rule.operator}")


# =========================
# EVALUATE RULES ON TRAIN
# =========================
def evaluate_rules(df, rules):

    results = []

    for rule in rules:

        mask = apply_rule(df, rule)

        subset = df[mask]

        total = len(subset)

        if total == 0:
            winrate = 0.0
        else:
            winrate = float(subset["IsWin"].mean())

        edge = winrate - 0.5

        results.append({
            "Rule": str(rule),
            "Feature": rule.feature,
            "Operator": rule.operator,
            "Threshold": rule.threshold,
            "Trades": total,
            "WinRate": winrate,
            "Edge": edge
        })

    result_df = pd.DataFrame(results)

    # 🔥 מיון לפי Edge (הכי חשוב למעלה)
    result_df = result_df.sort_values(by="Edge", ascending=False)

    return result_df