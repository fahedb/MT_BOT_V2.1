// ---------------------------------------------------------------------------
// Indicator extraction — docs/REFACTOR_PLAN.md section 7.7 / Phase 2.
//
// Turns a BarSeries into the channel Series the registry names. These are the
// C++ ports of what the Python VectorBuilders computed and what the legacy
// IndicatorStorage held.
//
// The difference from both: a value that does not exist yet is *missing*, not
// a sentinel and not a seeded guess. An EMA(50) has no value at bar 10, and
// saying so (via Series::pushMissing) is what lets the standardiser exclude
// it instead of normalising a fabricated number (finding B6).
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>

#include "mt/base/error.h"
#include "mt/data/bar.h"
#include "mt/data/series.h"

namespace mt::features {

// --- raw price channels ---------------------------------------------------

[[nodiscard]] data::Series closeSeries(const data::BarSeries& bars);
[[nodiscard]] data::Series rangeSeries(const data::BarSeries& bars);  // high - low
[[nodiscard]] data::Series bodySeries(const data::BarSeries& bars);   // close - open

// Where the close sits inside the bar's own range, in [0, 1]. Zero-range bars
// are missing rather than 0.5, which would be an invented value.
[[nodiscard]] data::Series closeLocationSeries(const data::BarSeries& bars);

// --- indicators -----------------------------------------------------------

// Exponential moving average of the close. The first `period - 1` values are
// missing; the value at index period-1 is the simple mean of the first
// `period` closes, which is the conventional seed.
[[nodiscard]] Result<data::Series> ema(const data::BarSeries& bars, std::size_t period);

// Wilder's RSI, in [0, 100]. The first `period` values are missing.
[[nodiscard]] Result<data::Series> rsi(const data::BarSeries& bars, std::size_t period);

// Wilder's average true range, which is what slope channels are normalised by
// (plan section 6.2 step 4).
[[nodiscard]] Result<data::Series> atr(const data::BarSeries& bars, std::size_t period);

// --- derived channels -----------------------------------------------------

// First difference. Index 0 is missing: there is no prior sample to difference
// against, and the legacy code's choice of 0.0 there is a fabricated value
// that every window containing index 0 then normalised against.
[[nodiscard]] data::Series difference(const data::Series& source);

// a - b, elementwise; missing wherever either side is missing.
[[nodiscard]] Result<data::Series> subtract(const data::Series& a, const data::Series& b,
                                            std::string name);

// a / b, missing wherever either side is missing or b is ~0.
[[nodiscard]] Result<data::Series> divide(const data::Series& a, const data::Series& b,
                                          std::string name);

// Rolling percentile rank of each sample within the preceding `lookback`
// samples, in [0, 1]. Used for the volatility band in the regime prefilter
// (plan section 7.2 stage 0) because it is scale-free and comparable across
// years.
[[nodiscard]] Result<data::Series> rollingPercentileRank(const data::Series& source,
                                                         std::size_t lookback);

}  // namespace mt::features
