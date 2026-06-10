import numpy as np


def extract(pattern):

    candles = pattern["candles"]

    if len(candles) < 20:
        return None

    closes = [float(c["close"]) for c in candles]

    returns = []

    for i in range(1, len(closes)):
        prev_close = closes[i - 1]
        curr_close = closes[i]

        if prev_close == 0:
            return None

        ret = (curr_close - prev_close) / prev_close
        returns.append(ret)

    if len(returns) < 20:
        return None

    vol_5 = np.std(returns[-5:])
    vol_20 = np.std(returns[-20:])

    return [
        float(vol_5),
        float(vol_20)
    ]