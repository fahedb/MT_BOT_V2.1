def run_model(csv_path, simulation_id, stat_type):

    import pandas as pd
    from sklearn.ensemble import RandomForestClassifier

    print(f"\n=== Running model | {stat_type} | {csv_path} ===")

    # =========================
    # 1. Load data
    # =========================
    df = pd.read_csv(csv_path)

    # =========================
    # 2. Dynamic Features
    # =========================
    if stat_type == "Combined":

        feature_cols = [
            # General
            "G_Mean", "G_Up", "G_Down", "G_UpPct", "G_DownPct", "G_Votes",

            # First
            "F_Mean", "F_Up", "F_Down", "F_UpPct", "F_DownPct", "F_Votes",

            # Highest
            "H_Mean", "H_Up", "H_Down", "H_UpPct", "H_DownPct", "H_Votes",

            # 🔥 engineered
            "GF_UpDiff",
            "GH_UpDiff",
            "Momentum",
            "Strength_G",
            "Strength_F",
            "Strength_H"
        ]

    else:
        feature_cols = [
            'Mean',
            'PureMean_Up',
            'PureMean_Down',
            'UpPercentage',
            'DownPercentage',
            'TotalVotes'
        ]

    X = df[feature_cols]

    # =========================
    # 3. Target
    # =========================
    y = df['IsWin']

    # =========================
    # 4. Time split
    # =========================
    split_index = int(len(X) * 0.7)

    X_train = X.iloc[:split_index]
    X_test  = X.iloc[split_index:]

    y_train = y.iloc[:split_index]
    y_test  = y.iloc[split_index:]

    # =========================
    # 5. Depths
    # =========================
    depths = [5, 8, 10, 12, 15, 20, None]

    all_results = []

    for depth in depths:

        print(f"\n--- Running depth: {depth} ---")

        model = RandomForestClassifier(
            n_estimators=100,
            max_depth=depth,
            n_jobs=-1,
            class_weight="balanced"
        )

        model.fit(X_train, y_train)

        accuracy = model.score(X_test, y_test)
        print("Accuracy:", round(accuracy, 4))

        # Feature importance
        print("Feature importance:")
        importance = {}
        for name, val in zip(feature_cols, model.feature_importances_):
            importance[name] = float(val)
            print(name, round(val, 3))

        probs = model.predict_proba(X_test)[:, 1]

        for t in [0.55, 0.6, 0.65, 0.7]:

            mask = probs > t
            total = int(mask.sum())

            if total < 50:
                continue

            winrate = float(y_test[mask].mean())

            print(f"Depth {depth} | Threshold {t}: Total={total}, WinRate={winrate:.2f}")

            all_results.append({
                "SimulationID": simulation_id,
                "ModelType": "RandomForest",
                "StatType": stat_type,
                "MaxDepth": -1 if depth is None else depth,
                "Accuracy": float(accuracy),
                "Threshold": t,
                "TotalTrades": total,
                "WinRate": winrate
            })

    return all_results