import numpy as np


# =========================
# RULE CLASS (future-proof)
# =========================
class Rule:
    def __init__(self, feature, operator, threshold):
        self.feature = feature
        self.operator = operator
        self.threshold = threshold

    def __repr__(self):
        return f"{self.feature} {self.operator} {round(self.threshold, 4)}"


# =========================
# GENERATOR
# =========================
def generate_rules(df, feature, percentiles=None):

    if percentiles is None:
        percentiles = [5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95]

    values = df[feature].values

    # 🔥 thresholds based on data distribution
    thresholds = np.percentile(values, percentiles)

    rules = []

    for t in thresholds:

        # >
        rules.append(Rule(feature, ">", float(t)))

        # <
        rules.append(Rule(feature, "<", float(t)))

    print(f"Generated {len(rules)} rules for {feature}")

    return rules