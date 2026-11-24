// ---------------------------------------------------------------------------
// Decision policy — docs/REFACTOR_PLAN.md section 6.8.
//
// The only place allowed to decide a trade, and the only consumer of a Signal.
//
// The important difference from the legacy trade layer is that every quantity
// here has a unit. The old rule compared raw unbounded scores and percentages
// to hand-set constants (Pattern_Min_Percent_To_ExecuteOrder,
// Min_MeanPips_ToExecute_Order, ScoreToAct), which had to be retuned whenever
// the filter string or window length changed because the score's scale moved
// with them (finding S2).
//
// It also accounts for cost. Nothing in the pre-refactor trade layer
// subtracted spread or commission, which flatters every result it produced.
// ---------------------------------------------------------------------------
#pragma once

#include <string_view>

#include "mt/aggregate/aggregate.h"
#include "mt/base/error.h"

namespace mt::policy {

enum class RejectReason {
    None,
    NoSignal,            // match set was empty or unusable
    NoDirection,         // every match said NOISE
    ProbabilityTooLow,   // Wilson lower bound below the threshold
    EvidenceTooThin,     // effective sample size below the threshold
    MatchQualityTooLow,  // mean calibrated score below the threshold
    TooNoisy,            // noise ratio above the threshold
    EdgeBelowCost,       // expected value does not clear cost plus margin
};

[[nodiscard]] std::string_view toString(RejectReason reason) noexcept;

struct PolicyConfig {
    // --- evidence gates, all in their own units ---
    double minProbabilityLowerBound = 0.55;  // probability, not a percentage
    double minEffectiveSampleSize = 8.0;     // independent observations
    double minMeanScore = 70.0;              // calibrated score, 0..100
    double maxNoiseRatio = 0.40;             // share of the match set

    // --- economics, in points ---
    // Round-trip cost: spread plus commission, expressed in points. There is
    // no sensible default, because it is broker- and symbol-specific; leaving
    // it at zero reproduces the legacy behaviour of ignoring cost, which is
    // why it must be set explicitly in production config.
    double costPoints = 0.0;

    // Required expected value above cost before trading.
    double marginPoints = 0.0;

    // --- exits ---
    // When true, the stop and target come from the match set's own realised
    // excursions rather than from fixed offsets, which is the point of having
    // the statistics at all.
    bool deriveStopsFromMatches = true;
    double stopLossPoints = 150.0;
    double takeProfitPoints = 150.0;

    // Floor and ceiling applied to derived stops, so a freak match set cannot
    // produce an absurd exit.
    double minStopPoints = 20.0;
    double maxStopPoints = 1000.0;

    // --- sizing ---
    double baseLots = 0.01;
    bool scaleByConfidence = false;
    double maxLots = 1.0;
};

struct Decision {
    bool trade = false;
    labels::Direction direction = labels::Direction::Noise;

    double lots = 0.0;
    double stopLossPoints = 0.0;
    double takeProfitPoints = 0.0;

    // p * TP - (1 - p) * SL - cost, using the Wilson lower bound for p.
    double expectedEdgePoints = 0.0;

    RejectReason reason = RejectReason::NoSignal;
};

// Pure function: same signal and config always give the same decision, with no
// I/O and no hidden state. That is what makes the trade rule testable, which
// the legacy TradeLayerRun was not.
[[nodiscard]] Decision decide(const aggregate::Signal& signal, const PolicyConfig& config);

[[nodiscard]] Status validate(const PolicyConfig& config);

}  // namespace mt::policy
