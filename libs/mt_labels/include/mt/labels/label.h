// ---------------------------------------------------------------------------
// Forward labelling — docs/REFACTOR_PLAN.md section 6.7 / Phase 5.
//
// What happened after a pattern. This is the single most important modelling
// choice in the whole system, and in the pre-refactor engine it was ambiguous:
// StatisticsDecession computed three different labels (`first`, `general`,
// `highest`), stuffed all of them into DirectionStats, and the trade rule read
// a mixture (finding S9).
//
// Worse, the "first touch" label summed a precomputed per-bar Pip_Change, so
// it could not see intrabar highs and lows and therefore could not actually
// tell which side was touched first. The Python research path had already
// fixed this with first_hit_pips plus a same-candle policy; that definition is
// the canonical one here.
//
// One label is declared primary in config. The others are still computed, as
// secondary diagnostics, but nothing may decide a trade on a mixture.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <string_view>

#include "mt/base/error.h"
#include "mt/data/bar.h"

namespace mt::labels {

enum class Direction { Up, Down, Noise };

[[nodiscard]] std::string_view toString(Direction direction) noexcept;

// When a single bar's range covers both the up and the down barrier, OHLC
// cannot say which came first. Pretending to know is how a backtest acquires
// an edge that does not exist.
enum class SameCandlePolicy {
    Noise,      // the honest default: unknowable, so excluded
    UpFirst,    // optimistic for longs; only for sensitivity analysis
    DownFirst,  // the pessimistic counterpart
};

enum class LabelKind {
    FirstTouch,  // first barrier touched within the horizon (the default)
    NetReturn,   // close-to-close move over the whole horizon
    MaxFavourableExcursion,
};

struct LabelConfig {
    // Barrier distance, in points of `pipSize`. Symmetric by design: an
    // asymmetric barrier bakes a directional bias into the label itself.
    double touchPoints = 150.0;
    double pointSize = 0.01;

    std::size_t horizonBars = 15;
    SameCandlePolicy sameCandlePolicy = SameCandlePolicy::Noise;

    // Which label the policy layer is allowed to act on.
    LabelKind primary = LabelKind::FirstTouch;

    // A horizon that runs past a data gap is not a horizon. When the gap
    // between consecutive bars exceeds this, the label is invalid rather than
    // being computed across a weekend.
    Timestamp maxBarGapMs = 5 * 60 * 1000;
};

struct ForwardLabel {
    Direction firstTouch = Direction::Noise;
    Direction netDirection = Direction::Noise;

    double netReturnPoints = 0.0;   // close at horizon minus entry close
    double maxFavourablePoints = 0.0;
    double maxAdversePoints = 0.0;

    // Bars until the first barrier was touched; horizonBars when neither was.
    std::size_t barsToTouch = 0;

    // Both barriers fell inside one bar's range, so the order is unknowable.
    bool sameCandleAmbiguity = false;

    bool valid = false;

    // The direction the primary label reports, which is the only one the
    // policy layer may read.
    [[nodiscard]] Direction primaryDirection(LabelKind kind) const noexcept;
};

// Labels the window that *ends* at `entryIndex`: the forward horizon starts at
// entryIndex + 1, so nothing in the label can overlap the pattern. The legacy
// code did the equivalent by adding the current window length to the match
// index, which was right, but nothing enforced it.
[[nodiscard]] Result<ForwardLabel> labelForward(const data::BarSeries& bars,
                                                std::size_t entryIndex,
                                                const LabelConfig& config);

// --- leakage control ------------------------------------------------------

// The number of bars after a pattern that must not be used as another
// pattern's data, because their outcome is already part of this pattern's
// label. Phase 8's purged walk-forward needs the same number as its embargo.
[[nodiscard]] constexpr std::size_t embargoBars(const LabelConfig& config,
                                                std::size_t windowLength) noexcept {
    return config.horizonBars + windowLength;
}

// True when a candidate window may be used against a current window ending at
// `currentEndIndex`: it must end early enough that neither its data nor its
// forward horizon reaches into the present (finding B12).
[[nodiscard]] constexpr bool isCandidateAllowed(std::size_t candidateEndIndex,
                                                std::size_t currentEndIndex,
                                                const LabelConfig& config,
                                                std::size_t windowLength) noexcept {
    const std::size_t required = embargoBars(config, windowLength);
    return candidateEndIndex + required <= currentEndIndex;
}

}  // namespace mt::labels
