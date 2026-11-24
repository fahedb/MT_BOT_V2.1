// ---------------------------------------------------------------------------
// Window standardisation — docs/REFACTOR_PLAN.md section 6.2.
//
// Turns a raw channel window into a comparable *shape*, removing the things
// similarity should ignore (price level, volatility level) and nothing else.
//
// What this replaces, and why:
//
//  * Per-window min-max normalisation of values and nothing at all for slopes
//    (findings S1, S5). Min-max is decided by two samples, so one spike
//    rescales the whole window; and leaving slopes raw means an H4 slope and
//    an M1 slope were compared on the same numeric footing.
//  * Median/MAD with clamping is used instead: robust to outliers, and the
//    clamp bounds how much a single bar can move the result at all.
//  * Missing samples are excluded from the statistics rather than being
//    normalised as if they were data (finding B6).
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "mt/base/error.h"
#include "mt/data/series.h"

namespace mt::features {

enum class StandardiseMode {
    // x_hat = clamp((x - median) / (1.4826 * MAD), -clampSigma, +clampSigma)
    // The default. Level- and scale-invariant, outlier-resistant.
    RobustZ,

    // Mean/standard-deviation version. Offered for comparison in Phase 3's
    // benchmark; more sensitive to outliers than RobustZ.
    MeanZ,

    // (x - min) / (max - min). What the legacy MA kernel did. Kept only so the
    // old behaviour can be measured against the new one, not for production.
    MinMax,

    // Level removed, scale kept. For channels whose amplitude is meaningful in
    // its own units and already bounded, e.g. an RSI level.
    CentreOnly,

    // Passed through unchanged. For channels that are already dimensionless,
    // e.g. a direction score in [-1, 1].
    None,
};

struct StandardiseConfig {
    StandardiseMode mode = StandardiseMode::RobustZ;

    // Clamp in units of the robust sigma. 4.0 keeps essentially all genuine
    // variation while stopping a single print from dominating.
    float clampSigma = 4.0F;

    // Floor for the scale estimate, so a flat window does not divide by zero.
    // Per channel, because "flat" means different things for a price and for
    // an RSI.
    float epsilon = 1e-6F;

    // Piecewise aggregate approximation: reduce the window to this many
    // segment means. 0 disables it. Removes bar-level noise, makes the
    // comparison cheaper, and buys mild tolerance to phase shift
    // (plan section 6.2 step 5).
    std::size_t paaSegments = 0;

    // A window with less than this fraction of valid samples is unusable and
    // contributes nothing; its weight is redistributed (plan section 6.5).
    double minValidRatio = 0.95;
};

struct StandardisedWindow {
    // Length is paaSegments when set, otherwise the input window length.
    std::vector<float> values;

    // False when the window had too many missing samples, or no usable scale.
    // Callers must check: an unusable window must not be scored, not scored
    // as zero.
    bool usable = false;
};

// --- statistics over the valid samples only -------------------------------

[[nodiscard]] std::optional<float> medianOfValid(const data::WindowView& window);

// Median absolute deviation, scaled by 1.4826 so that for normally
// distributed data it estimates the same quantity as the standard deviation.
[[nodiscard]] std::optional<float> robustSigmaOfValid(const data::WindowView& window,
                                                      float median);

[[nodiscard]] std::optional<float> meanOfValid(const data::WindowView& window);

[[nodiscard]] std::optional<float> stdDevOfValid(const data::WindowView& window, float mean);

struct MinMaxOfValid {
    float min;
    float max;
};

// Note the contrast with the legacy Find_MaxMinValues, which seeded min at
// 1e6 and max at 0 and so reported max = 0 for any all-negative series
// (finding B7). This seeds from the first valid sample.
[[nodiscard]] std::optional<MinMaxOfValid> minMaxOfValid(const data::WindowView& window);

// --- piecewise aggregate approximation ------------------------------------

// Mean of each of `segments` equal-ish spans. Segment boundaries are computed
// so that the segments cover the window exactly, with the remainder spread
// over the earliest segments rather than dumped on the last one.
[[nodiscard]] Result<std::vector<float>> piecewiseAggregate(const std::vector<float>& values,
                                                            std::size_t segments);

// --- the main entry point -------------------------------------------------

[[nodiscard]] Result<StandardisedWindow> standardise(const data::WindowView& window,
                                                     const StandardiseConfig& config);

// Standardises `window` after first dividing it by `scale`, used for slope
// channels: the scale is a rolling volatility estimate for the same timeframe,
// so a slope is expressed in units of current volatility before its shape is
// compared (plan section 6.2 step 4). A non-positive scale is an error rather
// than a silent pass-through.
[[nodiscard]] Result<StandardisedWindow> standardiseScaled(const data::WindowView& window,
                                                           float scale,
                                                           const StandardiseConfig& config);

}  // namespace mt::features
