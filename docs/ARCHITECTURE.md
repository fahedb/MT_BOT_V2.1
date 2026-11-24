# Architecture

The living description of the system as it is now. For the reasoning behind it,
and for what was wrong with the previous design, see
[REFACTOR_PLAN.md](REFACTOR_PLAN.md).

## The idea

Take the last *N* bars. Find windows in history whose multi-timeframe shape is
most similar. Look at what price did after each of them. Aggregate those
outcomes into a directional expectation, and trade only when the expectation is
strong enough to clear costs.

Unchanged from the original project. What changed is how similarity is defined,
how it is scored, how candidates are retrieved, and how any of it is verified.

## Layers

Dependencies point one way, and CMake enforces it: a target may only link the
ones above it.

```
mt_base        Result/Status/Error, ILogger, IClock
   |
mt_data        Bar, BarSeries, Series + validity masks, WindowView
   |
mt_features    indicators, ChannelRegistry, ChannelSet, standardisation, PAA
   |
mt_similarity  distances, calibration, Scorer -> MatchScore
   |           +-- mt_labels    forward labelling, embargo rules
   |           |
mt_retrieval   +-- mt_aggregate ESS, shrinkage, Wilson bound -> Signal
   |           |
   |           +-- mt_policy    Signal -> Decision
   |                |
mt_engine ---------+   composes all of it
   |
apps/          mtcalibrate, mtbacktest
```

Rules for everything under `libs/`:

* no singletons, no global state, no I/O inside the maths;
* failures return `mt::Error`, never log-and-continue;
* `/W4` clean, and every target has tests;
* anything with a unit says so in its name (`...Points`, `...Bars`, `lots`).

## The per-bar pipeline

`mt::engine::PatternEngine::evaluateAt(index)`:

| Stage | What happens | Where |
|---|---|---|
| 0 | regime prefilter (volatility / trend / session buckets) | `mt_retrieval/search.h` |
| 1 | the present window becomes one embedding vector | `mt_retrieval/embedding.h` |
| 2 | approximate nearest neighbours, top-M | `HnswSearchIndex` |
| 6 | embargo as a hard index cutoff | derived from `mt_labels::embargoBars` |
| 3 | exact calibrated multi-channel rescoring of the survivors | `mt_similarity/score.h` |
| 4 | optional alignment refinement (+/- k bars, or banded DTW) | `mt_similarity/distance.h` |
| 5 | de-overlap, then aggregate into a signal | `mt_aggregate` |
| — | decide, with costs | `mt_policy` |

The stage numbers are the plan's, in section 7.1. Stages 3–5 live outside
`mt_retrieval` because they are scoring and aggregation concerns.

## The three ideas that carry the design

**Calibration.** A raw distance is in one channel's own units and says nothing
about how improbable a match is. Each channel's distance is therefore mapped
through its own empirical CDF, measured offline over random non-overlapping
window pairs:

```
p_c = F_c(d_c)      probability a random pair is at least this close
s_c = 1 - p_c       calibrated similarity, comparable across channels
```

That makes thresholds meaningful, makes channels comparable, and lets the
engine legitimately return nothing.

**A bounded, decomposable score.**

```
S = 100 * [ (1 - lambda) * SUM_c (w_c * s_c) + lambda * MIN_c (s_c) ]
```

with weights summing to one and assigned per correlation cluster, so redundant
channels share influence rather than multiplying it. The `min` term is a
consistency gate. `MatchScore` is immutable and keeps its per-channel
breakdown, so a score can always be explained — and there is no shared mutable
buffer for a later stage to zero.

**Evidence, not percentages.** Matches are de-overlapped, then weighted, and
the result reports an effective sample size, a shrunk probability and a Wilson
lower bound at that ESS. The policy layer decides on the bound, so a handful of
agreeing matches cannot look like certainty.

## Data flow

```
CSV / SQL / MT5 feed
   -> BarSeries                (validated: ordered, coherent OHLC)
   -> ChannelSet               (one Series per channel, with validity masks)
   -> StandardisedWindow[]     (median/MAD, clamped, PAA-reduced)
   -> EmbeddingStore           (one vector per window, built once)
   -> ISearchIndex             (hnswlib, or exact for reference)
   -> MatchScore               (calibrated, bounded, decomposable)
   -> ForwardLabel             (first touch of +/-X within K bars, intrabar)
   -> Signal                   (direction, probability, bound, ESS, confidence)
   -> Decision                 (trade, direction, lots, stops, expected edge)
```

## Artefacts

Produced offline, consumed by the engine, and versioned so a mismatch is
refused rather than silently used:

| Artefact | Built by | Checked against |
|---|---|---|
| calibration (`.bin`) | `mtcalibrate` | registry fingerprint |
| embeddings (`.bin`) | engine on build, or exported | dimension + row order |
| channel weights (`.json`) | research (Phase 3, pending data) | channel ids |

The registry fingerprint covers the channel set and its standardisation, but
deliberately not the weights: re-weighting does not invalidate a calibration,
whereas changing standardisation does.

## The legacy tree

`MTCore/`, `HTTPCore/`, `MTScanner/`, `MTDll/`, `MTAnalyzer/` and the rest are
still present and still build, as `mt_legacy_core` under
`-DMTBOT_BUILD_LEGACY_FULL=ON`. They remain the only path that talks to MT5 and
SQL Server. Phase 6 rewires the live loop onto `libs/`; Phase 9 deletes what is
left. See `cmake/LegacyTargets.cmake` for the two settings that turned out to
be load-bearing (`UNICODE`, and the CUDA include path even for CPU builds).

## Testing

| Suite | Command | What it guards |
|---|---|---|
| unit | `ctest --preset vs2022-debug -LE integration` | every library, plus the section 6.9 scoring contract |
| integration | `ctest --preset vs2022-debug -L integration` | the whole chain on a planted motif |
| golden | part of unit | the pre-refactor math has not drifted |
| characterisation | part of unit | the findings in plan section 4, as assertions |

The golden files in `tests/golden/baseline_v0/` pin the *old* scoring kernels.
They exist so the refactor is measurable, and CI fails if a build rewrites
them.
