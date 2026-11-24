// Tests for mt_policy: the decision rule, its gates, and the cost accounting
// that the legacy trade layer did not have.
#include <gtest/gtest.h>

#include "mt/policy/policy.h"

namespace {

mt::aggregate::Signal strongSignal() {
    mt::aggregate::Signal signal;
    signal.valid = true;
    signal.direction = mt::labels::Direction::Up;
    signal.probability = 0.72;
    signal.probabilityLowerBound = 0.65;
    signal.expectedMovePoints = 120.0;
    signal.moveDispersionPoints = 60.0;
    signal.effectiveSampleSize = 45.0;
    signal.agreement = 0.44;
    signal.noiseRatio = 0.10;
    signal.meanScore = 88.0;
    signal.confidence = 0.5;
    signal.matchCount = 45;
    return signal;
}

mt::policy::PolicyConfig permissiveConfig() {
    mt::policy::PolicyConfig config;
    config.minProbabilityLowerBound = 0.55;
    config.minEffectiveSampleSize = 8.0;
    config.minMeanScore = 70.0;
    config.maxNoiseRatio = 0.40;
    config.costPoints = 0.0;
    config.marginPoints = 0.0;
    return config;
}

}  // namespace

TEST(Policy, TradesWhenEveryGatePassesAndTheEdgeClearsCost) {
    const auto decision = mt::policy::decide(strongSignal(), permissiveConfig());
    EXPECT_TRUE(decision.trade);
    EXPECT_EQ(decision.direction, mt::labels::Direction::Up);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::None);
    EXPECT_GT(decision.lots, 0.0);
    EXPECT_GT(decision.expectedEdgePoints, 0.0);
}

TEST(Policy, AnInvalidSignalIsNeverATrade) {
    mt::aggregate::Signal signal = strongSignal();
    signal.valid = false;
    const auto decision = mt::policy::decide(signal, permissiveConfig());
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::NoSignal);
}

TEST(Policy, NoDirectionIsNeverATrade) {
    mt::aggregate::Signal signal = strongSignal();
    signal.direction = mt::labels::Direction::Noise;
    const auto decision = mt::policy::decide(signal, permissiveConfig());
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::NoDirection);
}

TEST(Policy, ThinEvidenceIsRejectedEvenWithAHighProbability) {
    // This is the behaviour the legacy engine lacked: a near-unanimous but
    // tiny match set produced a confident percentage and a trade (finding
    // S10).
    mt::aggregate::Signal signal = strongSignal();
    signal.effectiveSampleSize = 3.0;
    signal.probability = 0.95;
    signal.probabilityLowerBound = 0.90;  // pretend the bound were high

    const auto decision = mt::policy::decide(signal, permissiveConfig());
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::EvidenceTooThin);
}

TEST(Policy, PoorMatchQualityIsRejected) {
    mt::aggregate::Signal signal = strongSignal();
    signal.meanScore = 55.0;
    const auto decision = mt::policy::decide(signal, permissiveConfig());
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::MatchQualityTooLow);
}

TEST(Policy, TooMuchNoiseIsRejected) {
    mt::aggregate::Signal signal = strongSignal();
    signal.noiseRatio = 0.8;
    const auto decision = mt::policy::decide(signal, permissiveConfig());
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::TooNoisy);
}

TEST(Policy, DecidesOnTheLowerBoundNotThePointEstimate) {
    mt::aggregate::Signal signal = strongSignal();
    signal.probability = 0.85;             // looks compelling
    signal.probabilityLowerBound = 0.51;   // but the evidence is weak
    auto config = permissiveConfig();
    config.minProbabilityLowerBound = 0.60;

    const auto decision = mt::policy::decide(signal, config);
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::ProbabilityTooLow);
}

TEST(Policy, CostCanTurnAWinningProbabilityIntoANoTrade) {
    // Nothing in the legacy trade layer subtracted spread or commission, which
    // flatters every result it produced.
    const auto signal = strongSignal();

    auto free = permissiveConfig();
    EXPECT_TRUE(mt::policy::decide(signal, free).trade);

    auto expensive = permissiveConfig();
    expensive.costPoints = 500.0;  // absurd, to make the point unambiguous
    const auto decision = mt::policy::decide(signal, expensive);
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::EdgeBelowCost);
    EXPECT_LT(decision.expectedEdgePoints, 0.0);
}

TEST(Policy, MarginRequiresEdgeAboveCostNotMerelyPositive) {
    const auto signal = strongSignal();
    auto config = permissiveConfig();

    const auto baseline = mt::policy::decide(signal, config);
    ASSERT_TRUE(baseline.trade);

    config.marginPoints = baseline.expectedEdgePoints + 1.0;
    const auto decision = mt::policy::decide(signal, config);
    EXPECT_FALSE(decision.trade);
    EXPECT_EQ(decision.reason, mt::policy::RejectReason::EdgeBelowCost);
}

TEST(Policy, ExpectedEdgeUsesTheConfiguredExits) {
    const auto signal = strongSignal();
    auto config = permissiveConfig();
    config.deriveStopsFromMatches = false;
    config.takeProfitPoints = 100.0;
    config.stopLossPoints = 50.0;
    config.costPoints = 5.0;

    const auto decision = mt::policy::decide(signal, config);
    ASSERT_TRUE(decision.trade);
    EXPECT_DOUBLE_EQ(decision.takeProfitPoints, 100.0);
    EXPECT_DOUBLE_EQ(decision.stopLossPoints, 50.0);
    // 0.65 * 100 - 0.35 * 50 - 5 = 65 - 17.5 - 5 = 42.5
    EXPECT_NEAR(decision.expectedEdgePoints, 42.5, 1e-9);
}

TEST(Policy, DerivedExitsComeFromTheMatchSetAndAreClamped) {
    mt::aggregate::Signal signal = strongSignal();
    signal.expectedMovePoints = 5.0;  // tiny
    signal.moveDispersionPoints = 2.0;

    auto config = permissiveConfig();
    config.deriveStopsFromMatches = true;
    config.minStopPoints = 20.0;
    config.maxStopPoints = 400.0;

    const auto decision = mt::policy::decide(signal, config);
    EXPECT_GE(decision.takeProfitPoints, 20.0) << "clamped up to the floor";
    EXPECT_GE(decision.stopLossPoints, 20.0);

    signal.expectedMovePoints = 100000.0;
    const auto huge = mt::policy::decide(signal, config);
    EXPECT_LE(huge.takeProfitPoints, 400.0) << "clamped down to the ceiling";
}

TEST(Policy, ConfidenceScalingCanSizeDownButNeverBelowZero) {
    mt::aggregate::Signal signal = strongSignal();
    auto config = permissiveConfig();
    config.baseLots = 0.10;
    config.scaleByConfidence = true;

    signal.confidence = 0.5;
    const auto half = mt::policy::decide(signal, config);
    ASSERT_TRUE(half.trade);
    EXPECT_NEAR(half.lots, 0.05, 1e-12);

    signal.confidence = 0.0;
    const auto none = mt::policy::decide(signal, config);
    EXPECT_FALSE(none.trade) << "a zero-size position is a no-trade, not a trade";
}

TEST(Policy, IsAPureFunction) {
    const auto signal = strongSignal();
    const auto config = permissiveConfig();
    const auto first = mt::policy::decide(signal, config);
    for (int i = 0; i < 5; ++i) {
        const auto again = mt::policy::decide(signal, config);
        EXPECT_EQ(first.trade, again.trade);
        EXPECT_EQ(first.direction, again.direction);
        EXPECT_DOUBLE_EQ(first.lots, again.lots);
        EXPECT_DOUBLE_EQ(first.expectedEdgePoints, again.expectedEdgePoints);
    }
}

TEST(Policy, ValidatesItsConfiguration) {
    EXPECT_TRUE(mt::policy::validate(permissiveConfig()).ok());

    auto badProbability = permissiveConfig();
    badProbability.minProbabilityLowerBound = 1.5;
    EXPECT_FALSE(mt::policy::validate(badProbability).ok());

    auto badScore = permissiveConfig();
    badScore.minMeanScore = 200.0;
    EXPECT_FALSE(mt::policy::validate(badScore).ok());

    auto badCost = permissiveConfig();
    badCost.costPoints = -1.0;
    EXPECT_FALSE(mt::policy::validate(badCost).ok());

    auto badLots = permissiveConfig();
    badLots.baseLots = 0.0;
    EXPECT_FALSE(mt::policy::validate(badLots).ok());

    auto badStops = permissiveConfig();
    badStops.minStopPoints = 500.0;
    badStops.maxStopPoints = 100.0;
    EXPECT_FALSE(mt::policy::validate(badStops).ok());
}

TEST(Policy, RejectReasonNamesAreStable) {
    // These appear in logs and in the backtest's rejection breakdown, so they
    // are part of the contract.
    EXPECT_EQ(mt::policy::toString(mt::policy::RejectReason::EvidenceTooThin),
              "evidence_too_thin");
    EXPECT_EQ(mt::policy::toString(mt::policy::RejectReason::EdgeBelowCost),
              "edge_below_cost");
    EXPECT_EQ(mt::policy::toString(mt::policy::RejectReason::None), "none");
}
