#!/usr/bin/env python3
"""Generate synthetic OHLC bars in the CSV format the apps read.

Exists so mtcalibrate and mtbacktest can be exercised without a SQL Server or
an MT5 terminal. The series is a random walk with a repeating motif planted in
it, each occurrence followed by a consistent drift, so the engine has something
real to find. It is NOT market data and says nothing about the strategy.

    python tools/make_synthetic_bars.py --out bars.csv --bars 20000

Columns: timestamp,open,high,low,close,volume
with timestamp in epoch milliseconds, matching what the engine expects.
"""
import argparse
import math


MOTIF = [0.0, -0.4, -0.9, -1.5, -2.2, -2.6, -2.8, -2.6,
         -2.2, -1.6, -1.0, -0.5, -0.1, 0.2, 0.4, 0.5]


class SplitMix64:
    """Same generator the C++ tests use, so results line up."""

    MASK = (1 << 64) - 1

    def __init__(self, seed):
        self.state = seed & self.MASK

    def next(self):
        self.state = (self.state + 0x9E3779B97F4A7C15) & self.MASK
        z = self.state
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & self.MASK
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & self.MASK
        return (z ^ (z >> 31)) & self.MASK

    def unit(self):
        return (self.next() >> 11) / float(1 << 53)

    def signed_unit(self):
        return self.unit() * 2.0 - 1.0


def generate(count, spacing, seed, base_price, drift_bars, plant):
    rng = SplitMix64(seed)
    level = base_price
    motif_cursor = len(MOTIF)
    follow_through = 0
    start_ms = 1_600_000_000_000  # an arbitrary fixed epoch, for reproducibility

    rows = []
    for i in range(count):
        if plant and i % spacing == 0 and i > 200 and i + len(MOTIF) + drift_bars + 5 < count:
            motif_cursor = 0

        if motif_cursor < len(MOTIF):
            target = base_price + MOTIF[motif_cursor] * 1.2
            step = (target - level) * 0.8 + rng.signed_unit() * 0.05
            motif_cursor += 1
            if motif_cursor == len(MOTIF):
                follow_through = drift_bars
        elif follow_through > 0:
            step = 0.55 + rng.signed_unit() * 0.08
            follow_through -= 1
        else:
            step = rng.signed_unit() * 0.35

        open_price = level
        level += step
        close_price = level
        wick = 0.12 + abs(rng.signed_unit()) * 0.1
        high = max(open_price, close_price) + wick
        low = min(open_price, close_price) - wick

        rows.append((
            start_ms + i * 60_000,
            open_price,
            high,
            low,
            close_price,
            1.0,
        ))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", required=True, help="output CSV path")
    parser.add_argument("--bars", type=int, default=20000, help="number of bars")
    parser.add_argument("--spacing", type=int, default=300,
                        help="bars between planted motifs")
    parser.add_argument("--seed", type=int, default=0xBEEF, help="PRNG seed")
    parser.add_argument("--price", type=float, default=1800.0, help="base price level")
    parser.add_argument("--drift-bars", type=int, default=10,
                        help="length of the post-motif drift")
    parser.add_argument("--no-motif", action="store_true",
                        help="pure random walk, for a null comparison")
    args = parser.parse_args()

    if args.bars < 500:
        parser.error("--bars should be at least 500 for the indicators to warm up")

    rows = generate(args.bars, args.spacing, args.seed, args.price, args.drift_bars,
                    not args.no_motif)

    with open(args.out, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("timestamp,open,high,low,close,volume\n")
        for ts, o, h, low, c, v in rows:
            handle.write("%d,%.5f,%.5f,%.5f,%.5f,%.1f\n" % (ts, o, h, low, c, v))

    kind = "random walk (no motif)" if args.no_motif else "random walk + planted motif"
    print("wrote %d bars to %s (%s)" % (len(rows), args.out, kind))


if __name__ == "__main__":
    main()
