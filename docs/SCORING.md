# Scoring — the math as shipped

Exact definitions of what the engine computes, so a result can be reproduced
and an argument about it can be settled by reading rather than guessing.

For why it is built this way, and what the previous design did instead, see
[REFACTOR_PLAN.md](REFACTOR_PLAN.md) sections 3 (old math), 4 (findings) and 6
(design). Every formula below has a corresponding test; the file is named
beside it.

Notation: a *channel* `c` is one comparable series — (indicator, length,
timeframe, value-or-slope) or any derived feature. `L` is the pattern window
length in bars, `P` the number of PAA segments.

---

## 1. Standardisation

`mt_features::standardise`, tested in `tests/unit/test_features.cpp`.

For a window `x[0..L-1]` with validity bits `v[0..L-1]`:

**1.1 Usability.** If `(#valid / L) < minValidRatio` (default 0.95) the window
is *unusable*. It is not scored, and its weight is redistributed. An unusable
window is never scored as zero.

**1.2 Robust centring and scaling** (`RobustZ`, the default), over valid
samples only:

```
mu    = median(x)
sigma = 1.4826 * median(|x - mu|)          MAD, scaled to estimate sigma
x_hat = clamp((x - mu) / max(sigma, eps), -clampSigma, +clampSigma)
```

`clampSigma` defaults to 4. Arithmetic is in double even though the inputs are
float32; see §6 on precision. If `sigma < eps` the window is flat and `x_hat`
is all zeros — flat is the honest answer, not an amplification of noise.

Other modes exist for comparison and for channels that need them: `MeanZ`
(mean/sd), `MinMax` (what the legacy MA kernel did, kept only so the old
behaviour can be measured), `CentreOnly` (level removed, scale kept — for
bounded channels like an RSI level) and `None` (already dimensionless).

**1.3 Slope channels** are divided by a volatility estimate *before*
standardising (`standardiseScaled`), so a slope is expressed in units of
current volatility. Without this an H4 slope and an M1 slope are not
comparable, which is finding S5.

**1.4 PAA.** If `paaSegments = P > 0`, `x_hat` is reduced to `P` segment means
with boundaries at exact proportions, so every sample lands in exactly one
segment and none is empty. This removes bar-level noise, cuts cost, and buys
mild tolerance to phase shift.

---

## 2. Per-channel distance

`mt_similarity::distance`, tested in `tests/unit/test_similarity.cpp`. All
operate on standardised windows, so they measure shape and nothing else.

| Kind | Definition | Range |
|---|---|---|
| `L1Shape` (default) | `mean(abs(a - b))` | `[0, 2*clampSigma]` |
| `L2Shape` | `sqrt(mean((a - b)^2))` | same |
| `Correlation` | `(1 - pearson(a, b)) / 2`; `1.0` if either side is constant | `[0, 1]` |
| `Cosine` | `(1 - cos(a, b)) / 2` | `[0, 1]` |
| `DtwBand` | banded DTW cost divided by path length, band `w` | `>= 0` |

Each is zero for identical input, symmetric, and monotone in disagreement —
the properties `RelativeScoreCalc` lacked (finding S1). Equal lengths are a
precondition for the point-wise measures; a mismatch is an error, not a silent
prefix comparison.

**Alignment.** `bestShiftDistance(history, current, kind, maxShift)` tries
shifts in `[-maxShift, +maxShift]`, compares only the overlap, normalises by
it, and returns both the best distance and the shift. The shift is returned
because a persistently non-zero one means the window length or anchoring is
wrong.

---

## 3. Calibration

`mt_similarity::ChannelCalibration`, built by `apps/mtcalibrate`.

Offline, per channel, over `N` random **non-overlapping** window pairs
(default 200,000; pairs closer than `L + minSeparation` are rejected, because
an overlapping pair is trivially similar and would bias the null towards small
distances):

```
F_c = empirical CDF of d_c over those pairs, stored as 1024 quantile bins
p_c = F_c(d_c)        linearly interpolated, clamped to [0, 1]
s_c = 1 - p_c         calibrated similarity
```

`s_c = 0.99` means "closer than 99% of random pairs" and means the same thing
on every channel. This is what makes thresholds expressible as statements about
rarity, and what lets the engine return nothing.

A calibration records the registry fingerprint — a hash over channel ids,
distance kinds and standardisation settings, deliberately **not** weights — and
is refused if it does not match. Re-weighting does not invalidate a
calibration; changing standardisation does.

---

## 4. Match score

`mt_similarity::Scorer::score`, contract in
`tests/unit/test_scoring_acceptance.cpp`.

**4.1 Weights.** From the registry, normalised per correlation cluster: each
cluster's claim is the *mean* of its members' weights, cluster shares are
normalised to sum to 1, then each share is split among members in proportion
to their own weights. So fifteen near-identical EMA channels carry one
cluster's influence, not fifteen channels' worth (finding S4). A channel at
weight 0 is excluded entirely.

**4.2 Redistribution.** Let `U` be the set of channels usable on both sides and
`W_U = sum of w_c over U`. If `W_U < minUsableWeightRatio` (default 0.5) the
match is *not scored at all*. Otherwise effective weights are
`w'_c = w_c / W_U`, so the weighted mean stays on `[0, 1]` however many
channels dropped out.

**4.3 The score.**

```
S = 100 * [ (1 - lambda) * SUM_{c in U} (w'_c * s_c) + lambda * MIN_{c in U} s_c ]
```

`lambda` defaults to 0.25. The weighted mean is the main term; the `min` term
is a consistency gate, so a candidate that matches beautifully on nine channels
and contradicts on the tenth cannot win on volume.

**4.4 Hard gates.** A channel with `gate_c > 0` requires `s_c >= gate_c`. A
candidate failing any gate is dropped regardless of `S`.

**4.5 Properties**, all asserted: `S` in `[0, 100]`; `S = 100` for identity;
symmetric; invariant to level and positive scaling (to 0.01 points — see §6);
monotonically decreasing under added noise; a single 10-sigma bar moves `S` by
less than 15 points; `s_c` approximately uniform on random pairs (KS);
deterministic; independent of how many channels were scanned.

`MatchScore` is immutable and keeps every `s_c`, so any score can be explained.
There is no shared mutable buffer, which is why finding B1 cannot recur.

---

## 5. Labels, aggregation, decision

**5.1 Label** (`mt_labels::labelForward`, tested in `test_labels.cpp`). The
horizon starts at `entryIndex + 1`, so no part of a label overlaps the pattern
it describes. Walking forward at most `horizonBars`:

* `high >= close_entry + barrier` and not the other → `Up`
* `low <= close_entry - barrier` and not the other → `Down`
* both within one bar → `sameCandleAmbiguity`, resolved by `sameCandlePolicy`
  (default `Noise`: OHLC cannot order them, and pretending otherwise manufactures
  an edge)
* neither → `Noise`, with `barsToTouch = horizonBars`

A horizon spanning a bar gap larger than `maxBarGapMs` is **invalid**, not
computed across the weekend. Secondary outputs: net return, MFE, MAE.

**5.2 Leakage.** `embargoBars = horizonBars + L`. A candidate ending at `i` may
be used against a current window ending at `t` only if `i + embargoBars <= t`.
The engine derives this and hands it to the index as a hard cutoff, so no
caller can forget it (finding B12).

**5.3 Aggregation** (`mt_aggregate::aggregate`, tested in
`test_aggregate.cpp`). De-overlap first, keeping the best match per
`max(L, horizonBars)` neighbourhood. Then with
`w_m = (S_m / 100)^gamma` (default `gamma = 3`):

```
ESS   = (sum w_m)^2 / sum w_m^2
p_up  = (sum_{Up} w_m + alpha) / (sum_{directional} w_m + 2*alpha)     alpha = 2
bound = Wilson lower bound of p_dir at n = ESS, z = 1.645
```

`Noise` matches count towards `noiseRatio` but do not vote. Confidence is
`bound * ESS/(ESS+10) * meanScore/100 * (1 - noiseRatio)` — a product, so one
weak component cannot be averaged away.

**5.4 Decision** (`mt_policy::decide`, tested in `test_policy.cpp`). Gates in
order: `ESS >= minEffectiveSampleSize`, `meanScore >= minMeanScore`,
`noiseRatio <= maxNoiseRatio`, `bound >= minProbabilityLowerBound`. Then

```
edge = bound * takeProfit - (1 - bound) * stopLoss - cost
trade if edge > margin
```

The decision uses the **bound**, not the point estimate. `cost` must be set:
leaving it at zero reproduces the legacy behaviour of ignoring spread and
commission, which flatters every result.

---

## 6. Numerical precision

Standardisation centres and scales in double, so the only residual is the
float32 representation error of the inputs. That error is real and bounded:
shifting a ~1800 price series to ~1938, or scaling it to ~18000, moves every
sample onto a different float32 grid, and at 18000 the float32 step is ~0.002 —
a noticeable fraction of a typical window sigma.

Measured on the test fixture: level invariance holds to 2.2e-5 score points,
scale invariance to 1.8e-3. The acceptance tests therefore assert 0.01 points
rather than 1e-6, with those numbers recorded in the test. A genuine invariance
bug moves the score by whole points.

Golden-file comparisons use a 1e-5 relative tolerance, verified stable across
Debug and Release.

---

## 7. Defaults, and which are guesses

| Parameter | Default | Basis |
|---|---|---|
| `windowLength` | 48 | **Guess.** Needs the Phase 8 sensitivity sweep. |
| `paaSegments` | 12 | **Guess**, chosen as roughly `L/4`. |
| `clampSigma` | 4.0 | Convention: keeps essentially all genuine variation. |
| `minValidRatio` | 0.95 | **Guess**, deliberately strict. |
| `lambda` (min term) | 0.25 | **Guess.** Sweep it. |
| calibration pairs | 200,000 | Enough for 1024 bins to be stable. |
| `gamma` (match weight) | 3.0 | Carried over from the legacy power-law weighting. |
| `alpha` (shrinkage) | 2.0 | A 3-match set then cannot exceed ~0.71. |
| `wilsonZ` | 1.645 | One-sided 95%. |
| `minScore` | 70 | **Guess.** Means little until calibrated on real data. |
| `cost` | 0 | **Must be set.** Broker- and symbol-specific. |
| channel weights | uniform | **Not learned yet** — Phase 3, needs labelled history. |
| clusters | by indicator family | A prior, not a measurement. Phase 2's cluster tool is outstanding. |

The guesses are marked because they are guesses. None of them has been fitted,
and a result obtained at one setting is not a result until the sensitivity
sweep in plan section 8 says it survives the others.
