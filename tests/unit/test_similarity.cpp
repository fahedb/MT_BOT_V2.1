// Tests for mt_similarity: the distance functions, alignment search, and the
// calibration machinery. The end-to-end scoring contract lives in
// test_scoring_acceptance.cpp.
#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>

#include "mt/similarity/calibration.h"
#include "mt/similarity/distance.h"

namespace {

std::vector<float> ramp(std::size_t n, float step = 1.0F) {
    std::vector<float> out(n);
    for (std::size_t i = 0; i < n; ++i) {
        out[i] = static_cast<float>(i) * step;
    }
    return out;
}

}  // namespace

// --- distances are proper distances ---------------------------------------

TEST(Distance, IdenticalInputIsZero) {
    const auto a = ramp(32);
    EXPECT_DOUBLE_EQ(mt::similarity::l1Shape(a, a), 0.0);
    EXPECT_DOUBLE_EQ(mt::similarity::l2Shape(a, a), 0.0);
    EXPECT_NEAR(mt::similarity::correlationDistance(a, a), 0.0, 1e-12);
}

TEST(Distance, IsSymmetric) {
    const std::vector<float> a = {1, 2, 3, 4, 5};
    const std::vector<float> b = {5, 3, 1, 2, 4};

    EXPECT_DOUBLE_EQ(mt::similarity::l1Shape(a, b), mt::similarity::l1Shape(b, a));
    EXPECT_DOUBLE_EQ(mt::similarity::l2Shape(a, b), mt::similarity::l2Shape(b, a));
    EXPECT_DOUBLE_EQ(mt::similarity::correlationDistance(a, b),
                     mt::similarity::correlationDistance(b, a));
}

TEST(Distance, GrowsWithDisagreement) {
    const std::vector<float> base = {0, 1, 2, 3, 4};
    double previous = -1.0;
    for (const float offset : {0.0F, 0.5F, 1.0F, 2.0F, 5.0F}) {
        std::vector<float> shifted = base;
        for (float& v : shifted) {
            v += offset;
        }
        const double d = mt::similarity::l1Shape(base, shifted);
        EXPECT_GT(d, previous);
        previous = d;
    }
}

TEST(Distance, L1IsTheMeanAbsoluteDifference) {
    const std::vector<float> a = {0, 0, 0, 0};
    const std::vector<float> b = {1, 2, 3, 4};
    EXPECT_DOUBLE_EQ(mt::similarity::l1Shape(a, b), 2.5);
    EXPECT_NEAR(mt::similarity::l2Shape(a, b), std::sqrt(30.0 / 4.0), 1e-12);
}

TEST(Distance, UnlikeRelativeScoreCalcMagnitudeMatters) {
    // The legacy metric scored 1e-8 vs 2e-8 identically to 100 vs 200, because
    // it compared ratios (finding S1, proven in test_characterisation.cpp).
    // On standardised data a difference is a difference.
    const std::vector<float> tinyA = {1e-8F, 1e-8F, 1e-8F};
    const std::vector<float> tinyB = {2e-8F, 2e-8F, 2e-8F};
    const std::vector<float> largeA = {100.0F, 100.0F, 100.0F};
    const std::vector<float> largeB = {200.0F, 200.0F, 200.0F};

    EXPECT_LT(mt::similarity::l1Shape(tinyA, tinyB), 1e-7);
    EXPECT_GT(mt::similarity::l1Shape(largeA, largeB), 99.0);
}

TEST(Distance, CorrelationIgnoresAmplitudeButNotShape) {
    const std::vector<float> a = {1, 2, 3, 4, 5};
    std::vector<float> amplified = a;
    for (float& v : amplified) {
        v *= 10.0F;
    }
    EXPECT_NEAR(mt::similarity::correlationDistance(a, amplified), 0.0, 1e-6);

    const std::vector<float> reversed = {5, 4, 3, 2, 1};
    EXPECT_NEAR(mt::similarity::correlationDistance(a, reversed), 1.0, 1e-6);
}

TEST(Distance, ConstantWindowIsMaximallyDistantUnderCorrelation) {
    const std::vector<float> varying = {1, 2, 3, 4, 5};
    const std::vector<float> flat(5, 3.0F);
    // A flat line has no shape to correlate; claiming a match would be wrong.
    EXPECT_DOUBLE_EQ(mt::similarity::correlationDistance(varying, flat), 1.0);
}

TEST(Distance, RefusesLengthMismatchForPointwiseMeasures) {
    const auto a = ramp(10);
    const auto b = ramp(8);
    const auto result = mt::similarity::distance(a, b, mt::features::DistanceKind::L1Shape);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), mt::ErrorCode::InvalidArgument);
}

// --- dynamic time warping -------------------------------------------------

TEST(Dtw, ZeroForIdenticalSequences) {
    const auto a = ramp(20);
    const auto d = mt::similarity::dtwBandDistance(a, a, 3);
    ASSERT_TRUE(d) << d.error().describe();
    EXPECT_NEAR(d.value(), 0.0, 1e-12);
}

TEST(Dtw, ToleratesAPhaseShiftThatPointwiseDistancePunishes) {
    // A triangle wave and the same wave one sample late.
    std::vector<float> a(32);
    std::vector<float> b(32);
    for (std::size_t i = 0; i < 32; ++i) {
        a[i] = static_cast<float>(std::sin(static_cast<double>(i) * 0.4));
        b[i] = static_cast<float>(std::sin((static_cast<double>(i) - 1.0) * 0.4));
    }

    const double pointwise = mt::similarity::l1Shape(a, b);
    const auto warped = mt::similarity::dtwBandDistance(a, b, 3);
    ASSERT_TRUE(warped) << warped.error().describe();
    EXPECT_LT(warped.value(), pointwise)
        << "banded DTW should forgive a one-sample shift that L1 penalises";
}

TEST(Dtw, HandlesUnequalLengthsWhenTheBandAllowsIt) {
    const auto a = ramp(20);
    const auto b = ramp(24);
    // The band is widened internally to at least the length difference.
    const auto d = mt::similarity::dtwBandDistance(a, b, 2);
    ASSERT_TRUE(d) << d.error().describe();
    EXPECT_GE(d.value(), 0.0);
}

// --- alignment refinement -------------------------------------------------

TEST(BestShift, FindsTheOffsetAndReportsIt) {
    // history contains the current pattern starting three samples later.
    std::vector<float> current(20);
    for (std::size_t i = 0; i < current.size(); ++i) {
        current[i] = static_cast<float>(std::sin(static_cast<double>(i) * 0.3));
    }
    std::vector<float> history(26);
    for (std::size_t i = 0; i < history.size(); ++i) {
        history[i] = static_cast<float>(std::sin((static_cast<double>(i) - 3.0) * 0.3));
    }

    const auto aligned = mt::similarity::bestShiftDistance(
        history, current, mt::features::DistanceKind::L1Shape, 5);
    ASSERT_TRUE(aligned) << aligned.error().describe();

    EXPECT_EQ(aligned.value().shift, 3);
    EXPECT_NEAR(aligned.value().distance, 0.0, 1e-5);
}

TEST(BestShift, NeverWorseThanTheUnshiftedDistance) {
    std::vector<float> a(24);
    std::vector<float> b(24);
    for (std::size_t i = 0; i < 24; ++i) {
        a[i] = static_cast<float>(i % 7);
        b[i] = static_cast<float>((i + 2) % 7);
    }
    const double unshifted = mt::similarity::l1Shape(a, b);
    const auto aligned =
        mt::similarity::bestShiftDistance(a, b, mt::features::DistanceKind::L1Shape, 3);
    ASSERT_TRUE(aligned);
    EXPECT_LE(aligned.value().distance, unshifted + 1e-9);
}

TEST(BestShift, RejectsAShiftThatWouldLeaveNoOverlap) {
    const auto a = ramp(10);
    const auto b = ramp(10);
    EXPECT_FALSE(
        mt::similarity::bestShiftDistance(a, b, mt::features::DistanceKind::L1Shape, 10));
}

// --- calibration ----------------------------------------------------------

TEST(Calibration, MapsDistancesToTheirEmpiricalProbability) {
    // Uniform distances in [0, 1): F(d) should be approximately d.
    std::vector<float> samples;
    samples.reserve(10000);
    for (std::size_t i = 0; i < 10000; ++i) {
        samples.push_back(static_cast<float>(i) / 10000.0F);
    }
    const auto calibration = mt::similarity::ChannelCalibration::fromSamples(samples, 256);
    ASSERT_TRUE(calibration) << calibration.error().describe();

    EXPECT_NEAR(calibration.value().probabilityAtMost(0.25), 0.25, 0.01);
    EXPECT_NEAR(calibration.value().probabilityAtMost(0.50), 0.50, 0.01);
    EXPECT_NEAR(calibration.value().probabilityAtMost(0.90), 0.90, 0.01);

    // Similarity is the complement: a very small distance is a very rare match.
    EXPECT_NEAR(calibration.value().similarity(0.01), 0.99, 0.02);
}

TEST(Calibration, ClampsOutsideTheObservedRange) {
    std::vector<float> samples(2000);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        samples[i] = 1.0F + static_cast<float>(i) / 1000.0F;
    }
    const auto calibration = mt::similarity::ChannelCalibration::fromSamples(samples);
    ASSERT_TRUE(calibration);

    EXPECT_DOUBLE_EQ(calibration.value().probabilityAtMost(0.0), 0.0);
    EXPECT_DOUBLE_EQ(calibration.value().similarity(0.0), 1.0);
    EXPECT_DOUBLE_EQ(calibration.value().probabilityAtMost(1000.0), 1.0);
    EXPECT_DOUBLE_EQ(calibration.value().similarity(1000.0), 0.0);
}

TEST(Calibration, RefusesTooFewSamples) {
    const std::vector<float> few(10, 1.0F);
    const auto result = mt::similarity::ChannelCalibration::fromSamples(few);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), mt::ErrorCode::InvalidArgument);
}

TEST(Calibration, RejectsNonFiniteSamples) {
    std::vector<float> samples(2000, 1.0F);
    samples[7] = std::numeric_limits<float>::quiet_NaN();
    const auto result = mt::similarity::ChannelCalibration::fromSamples(samples);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), mt::ErrorCode::DataCorrupt);
}

TEST(Calibration, QuantileInvertsProbability) {
    std::vector<float> samples;
    samples.reserve(5000);
    for (std::size_t i = 0; i < 5000; ++i) {
        samples.push_back(static_cast<float>(i) / 5000.0F);
    }
    const auto calibration = mt::similarity::ChannelCalibration::fromSamples(samples, 512);
    ASSERT_TRUE(calibration);

    const double d = calibration.value().quantile(0.75);
    EXPECT_NEAR(calibration.value().probabilityAtMost(d), 0.75, 0.01);
}

TEST(CalibrationSet, RoundTripsThroughDisk) {
    std::vector<float> samples(3000);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        samples[i] = static_cast<float>(i) * 0.001F;
    }
    const auto channel = mt::similarity::ChannelCalibration::fromSamples(samples, 128);
    ASSERT_TRUE(channel);

    mt::similarity::CalibrationSet original(0xABCDEF0123456789ULL);
    ASSERT_TRUE(original.set("ema.20.M1.value", channel.value()).ok());
    ASSERT_TRUE(original.set("rsi.14.M5.level", channel.value()).ok());

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "mtbot_calibration_roundtrip.bin";
    ASSERT_TRUE(original.save(path.string()).ok());

    const auto loaded = mt::similarity::CalibrationSet::load(path.string());
    ASSERT_TRUE(loaded) << loaded.error().describe();
    EXPECT_EQ(loaded.value().fingerprint(), original.fingerprint());
    EXPECT_EQ(loaded.value().size(), 2u);

    const auto reloaded = loaded.value().get("ema.20.M1.value");
    ASSERT_TRUE(reloaded);
    EXPECT_EQ(reloaded.value()->quantiles(), channel.value().quantiles());

    std::filesystem::remove(path);
}

TEST(CalibrationSet, RejectsACorruptFile) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "mtbot_calibration_corrupt.bin";
    {
        std::ofstream out(path, std::ios::binary);
        out << "not a calibration file at all";
    }
    const auto loaded = mt::similarity::CalibrationSet::load(path.string());
    ASSERT_FALSE(loaded);
    EXPECT_EQ(loaded.error().code(), mt::ErrorCode::DataCorrupt);
    std::filesystem::remove(path);
}

TEST(CalibrationSet, ValidateAgainstCatchesMissingChannels) {
    mt::features::ChannelRegistry registry;
    for (int i = 0; i < 3; ++i) {
        mt::features::ChannelSpec spec;
        spec.id = "c" + std::to_string(i);
        ASSERT_TRUE(registry.add(spec).ok());
    }

    std::vector<float> samples(2000, 0.5F);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        samples[i] = static_cast<float>(i) * 0.0005F;
    }
    const auto channel = mt::similarity::ChannelCalibration::fromSamples(samples, 64);
    ASSERT_TRUE(channel);

    mt::similarity::CalibrationSet set(registry.fingerprint());
    ASSERT_TRUE(set.set("c0", channel.value()).ok());
    ASSERT_TRUE(set.set("c1", channel.value()).ok());
    // c2 deliberately absent: scoring would silently drop it from the total.

    const mt::Status status = set.validateAgainst(registry);
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.error().code(), mt::ErrorCode::NotFound);
}

// --- pair sampling --------------------------------------------------------

TEST(SampleWindowPairs, NeverReturnsOverlappingPairs) {
    mt::similarity::CalibrationSamplingConfig config;
    config.pairCount = 5000;
    config.minSeparation = 10;

    const std::size_t windowLength = 48;
    const auto pairs = mt::similarity::sampleWindowPairs(5000, windowLength, config);
    ASSERT_TRUE(pairs) << pairs.error().describe();
    ASSERT_EQ(pairs.value().size(), 5000u);

    // An overlapping pair is trivially similar and would bias the null
    // distribution towards small distances, making every real match look
    // unremarkable (plan section 6.4).
    for (const auto& [i, j] : pairs.value()) {
        const std::size_t gap = (i > j) ? (i - j) : (j - i);
        EXPECT_GE(gap, windowLength + config.minSeparation);
    }
}

TEST(SampleWindowPairs, IsDeterministicForAGivenSeed) {
    mt::similarity::CalibrationSamplingConfig config;
    config.pairCount = 500;
    config.seed = 12345;

    const auto first = mt::similarity::sampleWindowPairs(2000, 48, config);
    const auto second = mt::similarity::sampleWindowPairs(2000, 48, config);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    EXPECT_EQ(first.value(), second.value());
}

TEST(SampleWindowPairs, FailsLoudlyWhenHistoryIsTooShort) {
    mt::similarity::CalibrationSamplingConfig config;
    config.pairCount = 1000;
    const auto pairs = mt::similarity::sampleWindowPairs(50, 48, config);
    ASSERT_FALSE(pairs);
    EXPECT_EQ(pairs.error().code(), mt::ErrorCode::InvalidArgument);
}
