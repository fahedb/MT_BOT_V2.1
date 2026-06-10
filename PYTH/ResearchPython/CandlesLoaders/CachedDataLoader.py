import os
import pickle

from CandlesLoaders.HistoryLoader import load_history as load_history_data
from CandlesLoaders.CurrentLoader import load_history as load_current_data


CACHE_DIR = "Cache"

HISTORY_CACHE_PATH = os.path.join(CACHE_DIR, "history_data.pkl")
CURRENT_CACHE_PATH = os.path.join(CACHE_DIR, "current_data.pkl")


def _save_pickle(path, data):
    os.makedirs(CACHE_DIR, exist_ok=True)

    with open(path, "wb") as f:
        pickle.dump(data, f, protocol=pickle.HIGHEST_PROTOCOL)


def _load_pickle(path):
    with open(path, "rb") as f:
        return pickle.load(f)


def load_history_cached(force_reload=False):

    if not force_reload and os.path.exists(HISTORY_CACHE_PATH):
        print("\n=== Loading HISTORY from cache ===")
        return _load_pickle(HISTORY_CACHE_PATH)

    print("\n=== Loading HISTORY from SQL ===")
    data = load_history_data()

    print("\n=== Saving HISTORY cache ===")
    _save_pickle(HISTORY_CACHE_PATH, data)

    return data


def load_current_cached(force_reload=False):

    if not force_reload and os.path.exists(CURRENT_CACHE_PATH):
        print("\n=== Loading CURRENT from cache ===")
        return _load_pickle(CURRENT_CACHE_PATH)

    print("\n=== Loading CURRENT from SQL ===")
    data = load_current_data()

    print("\n=== Saving CURRENT cache ===")
    _save_pickle(CURRENT_CACHE_PATH, data)

    return data