import numpy as np

TLLEN = 50


def build_pattern(df, time_to_index, tp):

    if tp not in time_to_index:
        return None

    idx = time_to_index[tp]

    end = idx + TLLEN

    if end >= len(df):
        return None

    return df.iloc[idx:end]["ClosePrice"].values


def build_patterns(df_matches,
                   df_current, map_current,
                   df_history, map_history):

    print("\n=== Building Patterns ===")

    results = []

    for i, row in df_matches.iterrows():

        curr_tp = row["CurrentTP"]
        match_tp = row["MatchTP"]

        curr_pattern = build_pattern(df_current, map_current, curr_tp)
        hist_pattern = build_pattern(df_history, map_history, match_tp)

        if curr_pattern is None or hist_pattern is None:
            continue

        results.append({
            "CurrentTP": curr_tp,
            "MatchTP": match_tp,
            "Score": row["Score"],
            "CurrPattern": curr_pattern,
            "HistPattern": hist_pattern
        })

        if i % 50000 == 0:
            print(f"Processed {i}")

    print(f"Valid patterns: {len(results)}")

    return results