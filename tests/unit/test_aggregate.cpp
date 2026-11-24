// Tests for mt_aggregate: de-overlapping, effective sample size, shrinkage and
// the Wilson lower bound — the fixes for finding S10.
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "mt/aggregate/aggregate.h"

namespace {

mt::aggregate::Match makeMatch(std::size_t endIndex, double score,
                               mt::labels::Direction direction, double netPoints = 50.0) {
    mt::aggregate::Match match;
    match.endIndex = endIndex;
    match.score = score;
    match.label.valid = true;
    match.label.firstTouch = direction;
    match.label.netReturnPoints = netPoints;
    return match;
}

mt::aggregate::AggregateConfig config(std::size_t dedupRadius = 0) {
    mt::aggregate::AggregateConfig c;
    c.dedupRadius = dedupRadius;
    return c;
}

}  // namespace

// --- de-overlapping -------------------------------------------------------

TEST(Deduplicate, CollapsesNeighbouringMatchesIntoOne) {
    // Five consecutive offsets of the same historical event. The legacy
    // engine counted these as five independent observations (finding S10).
    const std::vector<mt::aggregate::Match> matches = {
        makeMatch(1000, 90.0, mt::labels::Direction::Up),
        makeMatch(1001, 88.0, mt::labels::Direction::Up),
        makeMatch(1002, 95.0, mt::labels::Direction::Up),
        makeMatch(1003, 85.0, mt::labels::Direction::Up),
        makeMatch(1004, 80.0, mt::labels::Direction::Up),
    };

    const auto unique = mt::aggregate::deduplicate(matches, 48);
    ASSERT_EQ(unique.size(), 1u);
    EXPECT_EQ(unique.front().endIndex, 1002u) << "the strongest of the cluster survives";
}

TEST(Deduplicate, KeepsGenuinelySeparateEvents) {
    const std::vector<mt::aggregate::Match> matches = {
        makeMatch(1000, 90.0, mt::labels::Direction::Up),
        makeMatch(1100, 88.0, mt::labels::Direction::Up),
        makeMatch(1200, 92.0, mt::labels::Direction::Up),
    };

    const auto unique = mt::aggregate::deduplicate(matches, 48);
    EXPECT_EQ(unique.size(), 3u);
    // Chronological order out, which is what the caller expects for labelling.
    EXPECT_EQ(unique[0].endIndex, 1000u);
    EXPECT_EQ(unique[2].endIndex, 1200u);
}

TEST(Deduplicate, IsDeterministicOnTies) {
    const std::vector<mt::aggregate::Match> matches = {
        makeMatch(1000, 90.0, mt::labels::Direction::Up),
        makeMatch(1001, 90.0, mt::labels::Direction::Up),
    };
    const auto first = mt::aggregate::deduplicate(matches, 48);
    const auto second = mt::aggregate::deduplicate(matches, 48);
    ASSERT_EQ(first.size(), 1u);
    EXPECT_EQ(first.front().endIndex, second.front().endIndex);
    EXPECT_EQ(first.front().endIndex, 1000u) << "earliest index breaks the tie";
}

// --- effective sample size ------------------------------------------------

TEST(EffectiveSampleSize, EqualsCountWhenWeightsAreEqual) {
    const std::vector<double> weights(10, 0.5);
    EXPECT_NEAR(mt::aggregate::effectiveSampleSize(weights), 10.0, 1e-9);
}

TEST(EffectiveSampleSize, CollapsesWhenOneMatchDominates) {
    // One match carrying almost all the weight is one observation, however
    // many others are nominally present.
    std::vector<double> weights(20, 0.001);
    weights[0] = 10.0;
    const double ess = mt::aggregate::effectiveSampleSize(weights);
    EXPECT_LT(ess, 1.1);
    EXPECT_GT(ess, 0.9);
}

TEST(EffectiveSampleSize, IsZeroForNoWeight) {
    const std::vector<double> weights(5, 0.0);
    EXPECT_DOUBLE_EQ(mt::aggregate::effectiveSampleSize(weights), 0.0);
}

// --- Wilson bound ---------------------------------------------------------

TEST(WilsonLowerBound, PunishesSmallSamples) {
    // The same proportion, very different confidence. This is the property the
    // legacy percentage vote lacked entirely.
    const double small = mt::aggregate::wilsonLowerBound(0.7, 3.0, 1.645);
    const double medium = mt::aggregate::wilsonLowerBound(0.7, 30.0, 1.645);
    const double large = mt::aggregate::wilsonLowerBound(0.7, 300.0, 1.645);

    EXPECT_LT(small, medium);
    EXPECT_LT(medium, large);
    EXPECT_LT(small, 0.5) << "70% of three matches is not evidence of an edge";
    EXPECT_GT(large, 0.65);
}

TEST(WilsonLowerBound, StaysInRange) {
    for (const double p : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        for (const double n : {1.0, 10.0, 1000.0}) {
            const double bound = mt::aggregate::wilsonLowerBound(p, n, 1.645);
            EXPECT_GE(bound, 0.0);
            EXPECT_LE(bound, 1.0);
            EXPECT_LE(bound, p + 1e-9) << "a lower bound cannot exceed the estimate";
        }
    }
}

// --- aggregation ----------------------------------------------------------

TEST(Aggregate, EmptyMatchSetIsNotASignal) {
    const auto signal = mt::aggregate::aggregate({}, config());
    ASSERT_TRUE(signal) << signal.error().describe();
    EXPECT_FALSE(signal.value().valid)
        << "no match is a legitimate answer, not a neutral signal";
}

TEST(Aggregate, UnanimousLargeSetProducesAStrongSignal) {
    std::vector<mt::aggregate::Match> matches;
    for (int i = 0; i < 60; ++i) {
        matches.push_back(makeMatch(1000 + static_cast<std::size_t>(i) * 100, 95.0,
                                    mt::labels::Direction::Up, 80.0));
    }

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    ASSERT_TRUE(signal.value().valid);
    EXPECT_EQ(signal.value().direction, mt::labels::Direction::Up);
    EXPECT_GT(signal.value().probability, 0.9);
    EXPECT_GT(signal.value().probabilityLowerBound, 0.8);
    EXPECT_NEAR(signal.value().effectiveSampleSize, 60.0, 1e-6);
    EXPECT_NEAR(signal.value().expectedMovePoints, 80.0, 1e-6);
}

TEST(Aggregate, ThinSetCannotProduceConfidence) {
    // Three unanimous matches. The point estimate is high; the bound is not.
    std::vector<mt::aggregate::Match> matches = {
        makeMatch(1000, 95.0, mt::labels::Direction::Up),
        makeMatch(1200, 95.0, mt::labels::Direction::Up),
        makeMatch(1400, 95.0, mt::labels::Direction::Up),
    };

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    ASSERT_TRUE(signal.value().valid);
    EXPECT_EQ(signal.value().direction, mt::labels::Direction::Up);
    EXPECT_NEAR(signal.value().effectiveSampleSize, 3.0, 1e-6);
    // Shrinkage caps the estimate and the Wilson bound then cuts it further.
    EXPECT_LT(signal.value().probability, 0.75)
        << "the Beta prior must pull a 3-match set towards 0.5";
    EXPECT_LT(signal.value().probabilityLowerBound, 0.5);
}

TEST(Aggregate, OverlappingMatchesDoNotInflateEvidence) {
    // Twenty matches, but all within one dedup radius: one observation.
    std::vector<mt::aggregate::Match> matches;
    for (int i = 0; i < 20; ++i) {
        matches.push_back(makeMatch(1000 + static_cast<std::size_t>(i), 95.0,
                                    mt::labels::Direction::Up));
    }

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    EXPECT_EQ(signal.value().matchCount, 1u);
    EXPECT_NEAR(signal.value().effectiveSampleSize, 1.0, 1e-6);
    EXPECT_LT(signal.value().probabilityLowerBound, 0.4)
        << "twenty copies of one event must not look like twenty observations";
}

TEST(Aggregate, DisagreementLowersAgreementAndProbability) {
    std::vector<mt::aggregate::Match> matches;
    for (int i = 0; i < 20; ++i) {
        matches.push_back(makeMatch(1000 + static_cast<std::size_t>(i) * 100, 90.0,
                                    (i % 2 == 0) ? mt::labels::Direction::Up
                                                 : mt::labels::Direction::Down));
    }

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    ASSERT_TRUE(signal.value().valid);
    EXPECT_NEAR(signal.value().probability, 0.5, 0.05);
    EXPECT_LT(signal.value().agreement, 0.1);
    EXPECT_LT(signal.value().probabilityLowerBound, 0.5);
}

TEST(Aggregate, NoiseIsCountedButDoesNotVote) {
    std::vector<mt::aggregate::Match> matches;
    for (int i = 0; i < 10; ++i) {
        matches.push_back(makeMatch(1000 + static_cast<std::size_t>(i) * 100, 90.0,
                                    mt::labels::Direction::Up));
    }
    for (int i = 0; i < 10; ++i) {
        matches.push_back(makeMatch(5000 + static_cast<std::size_t>(i) * 100, 90.0,
                                    mt::labels::Direction::Noise));
    }

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    ASSERT_TRUE(signal.value().valid);
    EXPECT_EQ(signal.value().direction, mt::labels::Direction::Up);
    EXPECT_NEAR(signal.value().noiseRatio, 0.5, 1e-6);
    // Noise does not vote, so the directional probability stays high, but the
    // composite confidence is penalised for the mess.
    EXPECT_GT(signal.value().probability, 0.8);
    EXPECT_LT(signal.value().confidence, 0.5);
}

TEST(Aggregate, AllNoiseGivesNoDirection) {
    std::vector<mt::aggregate::Match> matches;
    for (int i = 0; i < 20; ++i) {
        matches.push_back(makeMatch(1000 + static_cast<std::size_t>(i) * 100, 90.0,
                                    mt::labels::Direction::Noise));
    }

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    EXPECT_TRUE(signal.value().valid);
    EXPECT_EQ(signal.value().direction, mt::labels::Direction::Noise);
    EXPECT_DOUBLE_EQ(signal.value().probability, 0.5);
}

TEST(Aggregate, HigherScoringMatchesCarryMoreWeight) {
    // One strong UP against three weak DOWNs. With a power-law weight the
    // strong match should dominate; with uniform weighting it should not.
    std::vector<mt::aggregate::Match> matches = {
        makeMatch(1000, 99.0, mt::labels::Direction::Up),
        makeMatch(1200, 40.0, mt::labels::Direction::Down),
        makeMatch(1400, 40.0, mt::labels::Direction::Down),
        makeMatch(1600, 40.0, mt::labels::Direction::Down),
    };

    auto powerLaw = config(48);
    powerLaw.weightMethod = mt::aggregate::WeightMethod::PowerLaw;
    powerLaw.gamma = 6.0;
    const auto weighted = mt::aggregate::aggregate(matches, powerLaw);
    ASSERT_TRUE(weighted);
    EXPECT_EQ(weighted.value().direction, mt::labels::Direction::Up);

    auto uniform = config(48);
    uniform.weightMethod = mt::aggregate::WeightMethod::Uniform;
    const auto unweighted = mt::aggregate::aggregate(matches, uniform);
    ASSERT_TRUE(unweighted);
    EXPECT_EQ(unweighted.value().direction, mt::labels::Direction::Down);
}

TEST(Aggregate, InvalidLabelsAreExcluded) {
    std::vector<mt::aggregate::Match> matches;
    for (int i = 0; i < 10; ++i) {
        matches.push_back(makeMatch(1000 + static_cast<std::size_t>(i) * 100, 90.0,
                                    mt::labels::Direction::Up));
    }
    // Matches at the right edge of history with no usable future.
    for (int i = 0; i < 10; ++i) {
        mt::aggregate::Match match = makeMatch(5000 + static_cast<std::size_t>(i) * 100, 99.0,
                                               mt::labels::Direction::Down);
        match.label.valid = false;
        matches.push_back(match);
    }

    const auto signal = mt::aggregate::aggregate(matches, config(48));
    ASSERT_TRUE(signal);
    EXPECT_EQ(signal.value().direction, mt::labels::Direction::Up);
    EXPECT_NEAR(signal.value().effectiveSampleSize, 10.0, 1e-6);
}

TEST(Aggregate, DispersionReflectsOutcomeSpread) {
    std::vector<mt::aggregate::Match> tight;
    std::vector<mt::aggregate::Match> wide;
    for (int i = 0; i < 20; ++i) {
        const auto index = 1000 + static_cast<std::size_t>(i) * 100;
        tight.push_back(makeMatch(index, 90.0, mt::labels::Direction::Up, 50.0));
        wide.push_back(makeMatch(index, 90.0, mt::labels::Direction::Up,
                                 (i % 2 == 0) ? 200.0 : -100.0));
    }

    const auto tightSignal = mt::aggregate::aggregate(tight, config(48));
    const auto wideSignal = mt::aggregate::aggregate(wide, config(48));
    ASSERT_TRUE(tightSignal);
    ASSERT_TRUE(wideSignal);
    EXPECT_NEAR(tightSignal.value().moveDispersionPoints, 0.0, 1e-6);
    EXPECT_GT(wideSignal.value().moveDispersionPoints, 100.0);
}
