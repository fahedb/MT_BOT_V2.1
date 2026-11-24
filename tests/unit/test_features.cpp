// Tests for mt_features: robust statistics, standardisation, PAA, and the
// channel registry's cluster-aware weighting (finding S4).
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "mt/data/series.h"
#include "mt/features/channel.h"
#include "mt/features/standardise.h"

namespace {

mt::data::WindowView viewOf(const mt::data::Series& series) {
    return mt::data::WindowView(series.values(), series.validity());
}

mt::data::Series seriesOf(const std::vector<float>& values) {
    mt::data::Series series("test");
    for (const float v : values) {
        series.push(v);
    }
    return series;
}

}  // namespace

// --- robust statistics ----------------------------------------------------

TEST(Standardise, MedianHandlesOddAndEvenCounts) {
    const auto odd = seriesOf({5.0F, 1.0F, 3.0F});
    EXPECT_FLOAT_EQ(*mt::features::medianOfValid(viewOf(odd)), 3.0F);

    const auto even = seriesOf({1.0F, 2.0F, 3.0F, 4.0F});
    EXPECT_FLOAT_EQ(*mt::features::medianOfValid(viewOf(even)), 2.5F);
}

TEST(Standardise, StatisticsIgnoreMissingSamples) {
    mt::data::Series series("mixed");
    series.push(1.0F);
    series.pushMissing();   // would have been -1.0f in the legacy arrays
    series.push(3.0F);
    series.push(5.0F);

    // Median of {1, 3, 5}, not of {1, 0, 3, 5} and not of {1, -1, 3, 5}.
    EXPECT_FLOAT_EQ(*mt::features::medianOfValid(viewOf(series)), 3.0F);
}

TEST(Standardise, MinMaxIsSeededFromDataNotFromConstants) {
    // Finding B7: the legacy Find_MaxMinValues seeded min at 1e6 and max at 0,
    // so every all-negative series reported max = 0.
    const auto negatives = seriesOf({-5.0F, -3.0F, -9.0F, -1.5F, -7.0F});
    const auto result = mt::features::minMaxOfValid(viewOf(negatives));
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result->min, -9.0F);
    EXPECT_FLOAT_EQ(result->max, -1.5F);

    const auto large = seriesOf({2.0e6F, 3.0e6F, 4.0e6F});
    const auto bigResult = mt::features::minMaxOfValid(viewOf(large));
    ASSERT_TRUE(bigResult.has_value());
    EXPECT_FLOAT_EQ(bigResult->min, 2.0e6F);
    EXPECT_FLOAT_EQ(bigResult->max, 4.0e6F);
}

TEST(Standardise, RobustSigmaIsMadScaledToSigma) {
    // Deviations from the median of 3 are {2,1,0,1,2}; their median is 1, so
    // the robust sigma is 1.4826.
    const auto series = seriesOf({1.0F, 2.0F, 3.0F, 4.0F, 5.0F});
    const float median = *mt::features::medianOfValid(viewOf(series));
    EXPECT_FLOAT_EQ(median, 3.0F);
    EXPECT_NEAR(*mt::features::robustSigmaOfValid(viewOf(series), median), 1.4826F, 1e-4);
}

// --- piecewise aggregate approximation ------------------------------------

TEST(Paa, SegmentsCoverEveryInputSampleExactlyOnce) {
    const std::vector<float> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const auto reduced = mt::features::piecewiseAggregate(values, 5);
    ASSERT_TRUE(reduced) << reduced.error().describe();
    ASSERT_EQ(reduced.value().size(), 5u);
    EXPECT_FLOAT_EQ(reduced.value()[0], 1.5F);
    EXPECT_FLOAT_EQ(reduced.value()[4], 9.5F);
}

TEST(Paa, HandlesIndivisibleLengths) {
    const std::vector<float> values = {1, 2, 3, 4, 5, 6, 7};
    const auto reduced = mt::features::piecewiseAggregate(values, 3);
    ASSERT_TRUE(reduced);
    ASSERT_EQ(reduced.value().size(), 3u);
    // Boundaries by exact proportion: [0,2) [2,4) [4,7).
    EXPECT_FLOAT_EQ(reduced.value()[0], 1.5F);
    EXPECT_FLOAT_EQ(reduced.value()[1], 3.5F);
    EXPECT_FLOAT_EQ(reduced.value()[2], 6.0F);
}

TEST(Paa, RefusesMoreSegmentsThanSamples) {
    const std::vector<float> values = {1, 2, 3};
    EXPECT_FALSE(mt::features::piecewiseAggregate(values, 4));
    EXPECT_FALSE(mt::features::piecewiseAggregate(values, 0));
}

// --- standardisation ------------------------------------------------------

TEST(Standardise, RobustZIsLevelAndScaleInvariant) {
    const auto base = seriesOf({10, 12, 11, 15, 13, 11, 14, 12});

    mt::features::StandardiseConfig config;
    config.mode = mt::features::StandardiseMode::RobustZ;

    const auto reference = mt::features::standardise(viewOf(base), config);
    ASSERT_TRUE(reference);
    ASSERT_TRUE(reference.value().usable);

    std::vector<float> shiftedValues;
    std::vector<float> scaledValues;
    for (std::size_t i = 0; i < base.size(); ++i) {
        shiftedValues.push_back(base[i] + 1000.0F);
        scaledValues.push_back(base[i] * 7.0F);
    }
    const auto shifted = mt::features::standardise(viewOf(seriesOf(shiftedValues)), config);
    const auto scaled = mt::features::standardise(viewOf(seriesOf(scaledValues)), config);
    ASSERT_TRUE(shifted);
    ASSERT_TRUE(scaled);

    for (std::size_t i = 0; i < reference.value().values.size(); ++i) {
        EXPECT_NEAR(reference.value().values[i], shifted.value().values[i], 1e-5);
        EXPECT_NEAR(reference.value().values[i], scaled.value().values[i], 1e-5);
    }
}

TEST(Standardise, ClampsOutliersRatherThanRescalingTheWindow) {
    std::vector<float> values(40, 100.0F);
    for (std::size_t i = 0; i < values.size(); ++i) {
        values[i] = 100.0F + static_cast<float>(i % 5);
    }
    values[20] = 100000.0F;  // one absurd print

    mt::features::StandardiseConfig config;
    config.mode = mt::features::StandardiseMode::RobustZ;
    config.clampSigma = 4.0F;

    const auto result = mt::features::standardise(viewOf(seriesOf(values)), config);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result.value().usable);

    // The spike is capped, and everything else keeps its ordinary magnitude.
    EXPECT_FLOAT_EQ(result.value().values[20], 4.0F);
    for (std::size_t i = 0; i < result.value().values.size(); ++i) {
        if (i == 20) continue;
        EXPECT_LT(std::fabs(result.value().values[i]), 4.0F);
    }
}

TEST(Standardise, FlatWindowIsFlatNotAmplified) {
    const std::vector<float> values(30, 42.0F);

    mt::features::StandardiseConfig config;
    config.mode = mt::features::StandardiseMode::RobustZ;

    const auto result = mt::features::standardise(viewOf(seriesOf(values)), config);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result.value().usable);
    for (const float v : result.value().values) {
        EXPECT_FLOAT_EQ(v, 0.0F);
    }
}

TEST(Standardise, TooManyMissingSamplesMakesTheWindowUnusable) {
    mt::data::Series series("gappy");
    for (int i = 0; i < 20; ++i) {
        if (i < 5) {
            series.pushMissing();
        } else {
            series.push(static_cast<float>(i));
        }
    }

    mt::features::StandardiseConfig config;
    config.minValidRatio = 0.95;

    const auto result = mt::features::standardise(viewOf(series), config);
    ASSERT_TRUE(result) << result.error().describe();
    // Not an error - a data gap is normal - but explicitly unusable, so the
    // caller drops it instead of scoring a zero.
    EXPECT_FALSE(result.value().usable);
}

TEST(Standardise, ScaledVariantRejectsNonPositiveScale) {
    const auto series = seriesOf({1, 2, 3, 4, 5});
    mt::features::StandardiseConfig config;

    EXPECT_FALSE(mt::features::standardiseScaled(viewOf(series), 0.0F, config));
    EXPECT_FALSE(mt::features::standardiseScaled(viewOf(series), -1.0F, config));
    EXPECT_TRUE(mt::features::standardiseScaled(viewOf(series), 2.0F, config));
}

// --- channel registry -----------------------------------------------------

TEST(ChannelRegistry, RejectsDuplicateIds) {
    mt::features::ChannelRegistry registry;
    mt::features::ChannelSpec spec;
    spec.id = "ema.20.M1.value";
    ASSERT_TRUE(registry.add(spec).ok());

    const mt::Status again = registry.add(spec);
    EXPECT_FALSE(again.ok());
    EXPECT_EQ(again.error().code(), mt::ErrorCode::AlreadyExists);
}

TEST(ChannelRegistry, WeightsNormaliseToOne) {
    mt::features::ChannelRegistry registry;
    for (int i = 0; i < 4; ++i) {
        mt::features::ChannelSpec spec;
        spec.id = "c" + std::to_string(i);
        spec.weight = static_cast<double>(i + 1);
        ASSERT_TRUE(registry.add(spec).ok());
    }

    const auto active = registry.normaliseWeights();
    ASSERT_TRUE(active) << active.error().describe();
    EXPECT_EQ(active.value(), 4u);

    double total = 0.0;
    for (const auto& spec : registry.specs()) {
        total += spec.weight;
    }
    EXPECT_NEAR(total, 1.0, 1e-12);
}

TEST(ChannelRegistry, ClustersStopRedundantChannelsFromOutvotingEverything) {
    // Finding S4: fifteen near-identical EMA channels should carry one
    // cluster's worth of influence, not fifteen channels' worth.
    mt::features::ChannelRegistry registry;
    for (int i = 0; i < 15; ++i) {
        mt::features::ChannelSpec spec;
        spec.id = "ema." + std::to_string(i);
        spec.cluster = "ema";
        spec.weight = 1.0;
        ASSERT_TRUE(registry.add(spec).ok());
    }
    mt::features::ChannelSpec rsi;
    rsi.id = "rsi.14";
    rsi.cluster = "rsi";
    rsi.weight = 1.0;
    ASSERT_TRUE(registry.add(rsi).ok());

    ASSERT_TRUE(registry.normaliseWeights());

    double emaTotal = 0.0;
    double rsiTotal = 0.0;
    for (const auto& spec : registry.specs()) {
        if (spec.cluster == "ema") emaTotal += spec.weight;
        if (spec.cluster == "rsi") rsiTotal += spec.weight;
    }

    // Two clusters, so each gets half, despite one having fifteen members.
    EXPECT_NEAR(emaTotal, 0.5, 1e-12);
    EXPECT_NEAR(rsiTotal, 0.5, 1e-12);
}

TEST(ChannelRegistry, ZeroWeightChannelsStayZeroAndArePruned) {
    mt::features::ChannelRegistry registry;
    for (int i = 0; i < 3; ++i) {
        mt::features::ChannelSpec spec;
        spec.id = "c" + std::to_string(i);
        spec.weight = (i == 1) ? 0.0 : 1.0;
        ASSERT_TRUE(registry.add(spec).ok());
    }

    const auto active = registry.normaliseWeights();
    ASSERT_TRUE(active);
    // This is how Phase 3's weight learning prunes the grid: a learned zero
    // removes the channel from scoring and from the embedding.
    EXPECT_EQ(active.value(), 2u);
    EXPECT_DOUBLE_EQ(registry[1].weight, 0.0);
    EXPECT_EQ(registry.embeddingChannels().size(), 2u);
}

TEST(ChannelRegistry, ApplyWeightsReportsUnknownChannels) {
    mt::features::ChannelRegistry registry;
    mt::features::ChannelSpec spec;
    spec.id = "known";
    ASSERT_TRUE(registry.add(spec).ok());

    // A learned weight for a channel that is not registered must not be
    // dropped silently: that would misreport which model is in use.
    const mt::Status status = registry.applyWeights({{"known", 2.0}, {"ghost", 3.0}});
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.error().code(), mt::ErrorCode::NotFound);
    EXPECT_DOUBLE_EQ(registry[0].weight, 2.0);  // the known one was still applied
}

TEST(ChannelRegistry, FingerprintTracksStandardisationNotWeights) {
    mt::features::ChannelRegistry a = mt::features::legacyGridRegistry(10);
    mt::features::ChannelRegistry b = mt::features::legacyGridRegistry(10);
    EXPECT_EQ(a.fingerprint(), b.fingerprint());

    // Re-weighting must not invalidate a calibration.
    ASSERT_TRUE(b.applyWeights({{"ema.10.M1.value", 0.0}}).ok());
    EXPECT_EQ(a.fingerprint(), b.fingerprint());

    // Changing the window shape must.
    const mt::features::ChannelRegistry c = mt::features::legacyGridRegistry(20);
    EXPECT_NE(a.fingerprint(), c.fingerprint());
}

TEST(ChannelRegistry, LegacyGridHasTheExpectedShape) {
    const mt::features::ChannelRegistry registry = mt::features::legacyGridRegistry();
    // 3 indicators x 5 lengths x 6 timeframes x 2 value types.
    EXPECT_EQ(registry.size(), 180u);
    // Six clusters: one per (indicator, value type).
    EXPECT_EQ(registry.clusters().size(), 6u);
    EXPECT_TRUE(registry.indexOf("ema.20.M15.value"));
    EXPECT_TRUE(registry.indexOf("rsi.10.H4.slope"));
}

TEST(ChannelRegistry, ResearchBaselineRegistryIsCompact) {
    const mt::features::ChannelRegistry registry = mt::features::researchBaselineRegistry();
    EXPECT_GT(registry.size(), 8u);
    EXPECT_LT(registry.size(), 20u);
    EXPECT_TRUE(registry.indexOf("returns.multi"));
    EXPECT_TRUE(registry.indexOf("h1.trend.state"));
}
