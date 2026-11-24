#include "mt/policy/policy.h"

#include <algorithm>
#include <cmath>

namespace mt::policy {

std::string_view toString(RejectReason reason) noexcept {
    switch (reason) {
        case RejectReason::None:               return "none";
        case RejectReason::NoSignal:           return "no_signal";
        case RejectReason::NoDirection:        return "no_direction";
        case RejectReason::ProbabilityTooLow:  return "probability_too_low";
        case RejectReason::EvidenceTooThin:    return "evidence_too_thin";
        case RejectReason::MatchQualityTooLow: return "match_quality_too_low";
        case RejectReason::TooNoisy:           return "too_noisy";
        case RejectReason::EdgeBelowCost:      return "edge_below_cost";
    }
    return "none";
}

Status validate(const PolicyConfig& config) {
    if (config.minProbabilityLowerBound < 0.0 || config.minProbabilityLowerBound > 1.0) {
        return Status(invalidArgument("PolicyConfig: minProbabilityLowerBound must be a "
                                      "probability in [0, 1]"));
    }
    if (config.minEffectiveSampleSize < 0.0) {
        return Status(invalidArgument("PolicyConfig: minEffectiveSampleSize must be >= 0"));
    }
    if (config.minMeanScore < 0.0 || config.minMeanScore > 100.0) {
        return Status(invalidArgument("PolicyConfig: minMeanScore must be in [0, 100]"));
    }
    if (config.maxNoiseRatio < 0.0 || config.maxNoiseRatio > 1.0) {
        return Status(invalidArgument("PolicyConfig: maxNoiseRatio must be in [0, 1]"));
    }
    if (config.costPoints < 0.0) {
        return Status(invalidArgument("PolicyConfig: costPoints must be >= 0"));
    }
    if (config.baseLots <= 0.0 || config.baseLots > config.maxLots) {
        return Status(invalidArgument("PolicyConfig: baseLots must be > 0 and <= maxLots"));
    }
    if (config.minStopPoints <= 0.0 || config.minStopPoints > config.maxStopPoints) {
        return Status(
            invalidArgument("PolicyConfig: minStopPoints must be > 0 and <= maxStopPoints"));
    }
    return Status::success();
}

Decision decide(const aggregate::Signal& signal, const PolicyConfig& config) {
    Decision decision;

    if (!signal.valid) {
        decision.reason = RejectReason::NoSignal;
        return decision;
    }
    if (signal.direction == labels::Direction::Noise) {
        decision.reason = RejectReason::NoDirection;
        return decision;
    }

    decision.direction = signal.direction;

    // Gates are checked in order of how fundamental they are, so the reported
    // reason is the most informative one rather than whichever came last.
    if (signal.effectiveSampleSize < config.minEffectiveSampleSize) {
        decision.reason = RejectReason::EvidenceTooThin;
        return decision;
    }
    if (signal.meanScore < config.minMeanScore) {
        decision.reason = RejectReason::MatchQualityTooLow;
        return decision;
    }
    if (signal.noiseRatio > config.maxNoiseRatio) {
        decision.reason = RejectReason::TooNoisy;
        return decision;
    }
    if (signal.probabilityLowerBound < config.minProbabilityLowerBound) {
        // Note: the *lower bound*, not the point estimate. A thin set with a
        // high point estimate fails here, which is the whole purpose.
        decision.reason = RejectReason::ProbabilityTooLow;
        return decision;
    }

    // --- exits -----------------------------------------------------------
    if (config.deriveStopsFromMatches) {
        // Target from the typical realised move; stop from its dispersion, so
        // a match set with wide outcomes gets a wider stop rather than being
        // shaken out of a trade its own history says is volatile.
        const double move = std::fabs(signal.expectedMovePoints);
        decision.takeProfitPoints =
            std::clamp(move, config.minStopPoints, config.maxStopPoints);
        decision.stopLossPoints = std::clamp(
            std::max(move * 0.75, signal.moveDispersionPoints), config.minStopPoints,
            config.maxStopPoints);
    } else {
        decision.takeProfitPoints = config.takeProfitPoints;
        decision.stopLossPoints = config.stopLossPoints;
    }

    // --- economics -------------------------------------------------------
    const double p = signal.probabilityLowerBound;
    decision.expectedEdgePoints = p * decision.takeProfitPoints -
                                  (1.0 - p) * decision.stopLossPoints - config.costPoints;

    if (decision.expectedEdgePoints <= config.marginPoints) {
        decision.reason = RejectReason::EdgeBelowCost;
        return decision;
    }

    // --- sizing ----------------------------------------------------------
    decision.lots = config.baseLots;
    if (config.scaleByConfidence) {
        decision.lots *= std::clamp(signal.confidence, 0.0, 1.0);
    }
    decision.lots = std::clamp(decision.lots, 0.0, config.maxLots);

    if (!(decision.lots > 0.0)) {
        // Confidence scaling can round a position to nothing; that is a
        // no-trade, not a zero-size trade.
        decision.trade = false;
        decision.reason = RejectReason::EdgeBelowCost;
        return decision;
    }

    decision.trade = true;
    decision.reason = RejectReason::None;
    return decision;
}

}  // namespace mt::policy
