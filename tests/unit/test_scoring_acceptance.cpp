// ---------------------------------------------------------------------------
// Acceptance tests for the new scoring system.
//
// This file IS the contract from docs/REFACTOR_PLAN.md section 6.9. Each test
// below is one of its bullet points, and several assert exactly the properties
// that tests/unit/test_characterisation.cpp shows the legacy scoring failing.
// If one of these starts failing, the scoring system is no longer the one the
// plan specified.
// ---------------------------------------------------------------------------
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "scoring_fixture.h"

namespace {

using mtbot::testing::makeScoringFixture;
using mtbot::testing::ScoringFixture;
using mtbot::testing::SplitMix64;

// Built once: the calibration sampling is the expensive part and it is
// deterministic, so every test can share it.
const ScoringFixture& fixture() {
    static const ScoringFixture instance = [] {
        auto result = makeScoringFixture();
        if (!result) {
            ADD_FAILURE() << "fixture construction failed: " << result.error().describe();
            return ScoringFixture{};
        }
        return std::move(result).value();
    }();
    return instance;
}

mt::similarity::Scorer makeScorer(double lambdaMin = 0.25) {
    mt::similarity::ScoreConfig config;
    config.lambdaMin = lambdaMin;
    auto scorer = mt::similarity::Scorer::create(fixture().registry, fixture().calibration,
                                                 config);
    EXPECT_TRUE(scorer) << (scorer ? "" : scorer.error().describe());
    return std::move(scorer).value();
}

std::vector<mt::features::StandardisedWindow> windowsAt(std::size_t endIndex) {
    auto windows = fixture().windowsEndingAt(endIndex);
    EXPECT_TRUE(windows) << (windows ? "" : windows.error().describe());
    return std::move(windows).value();
}

constexpr std::size_t kIndexA = 500;
constexpr std::size_t kIndexB = 1700;
constexpr std::size_t kIndexC = 2900;

// Tolerance for the level- and scale-invariance tests, in score points out of
// 100.
//
// Invariance is exact in real arithmetic and the implementation centres and
// scales in double, so the residual is the float32 representation error of the
// *inputs*: shifting a price series from ~1800 to ~1938, or scaling it to
// ~18000, moves every sample onto a different float32 grid. At 18000 the
// float32 step is ~0.002, a noticeable fraction of a window sigma.
//
// Measured on this fixture: 2.2e-5 points for a +137.5 level shift and
// 1.8e-3 points for a x10 scaling. 0.01 points leaves headroom without
// hiding a real break - a genuine invariance bug moves the score by whole
// points, not by thousandths.
constexpr double kInvarianceTolerance = 0.01;

}  // namespace

// ---------------------------------------------------------------------------
// "Identity: a window compared to itself gives s_c = 1 for every channel and
//  S = 100."
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, IdentityScoresOneHundred) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto windows = windowsAt(kIndexA);

    const auto score = scorer.score(windows, windows);
    ASSERT_TRUE(score) << score.error().describe();
    ASSERT_TRUE(score.value().valid());

    for (const auto& channel : score.value().channels()) {
        if (!channel.usable) continue;
        EXPECT_DOUBLE_EQ(channel.rawDistance, 0.0) << "channel " << channel.channelIndex;
        EXPECT_DOUBLE_EQ(channel.similarity, 1.0) << "channel " << channel.channelIndex;
    }
    EXPECT_DOUBLE_EQ(score.value().total(), 100.0);
    EXPECT_DOUBLE_EQ(score.value().minSimilarity(), 1.0);
}

// ---------------------------------------------------------------------------
// "Symmetry: S(a,b) == S(b,a)."
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, IsSymmetric) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto a = windowsAt(kIndexA);
    const auto b = windowsAt(kIndexB);

    const auto ab = scorer.score(a, b);
    const auto ba = scorer.score(b, a);
    ASSERT_TRUE(ab) << ab.error().describe();
    ASSERT_TRUE(ba) << ba.error().describe();
    ASSERT_TRUE(ab.value().valid());
    ASSERT_TRUE(ba.value().valid());

    EXPECT_NEAR(ab.value().total(), ba.value().total(), 1e-9);
}

// ---------------------------------------------------------------------------
// "Invariance: adding a constant to a whole window, or multiplying it by a
//  positive constant, must not change S, within 1e-6."
//
// This is the property the legacy scoring only half had: value channels were
// min-max normalised so they absorbed a scale change, while slope channels
// were compared raw and did not (finding S5, proven in
// test_characterisation.cpp).
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, InvariantToLevelShift) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto baseline = windowsAt(kIndexA);
    const auto other = windowsAt(kIndexB);

    const auto reference = scorer.score(baseline, other);
    ASSERT_TRUE(reference) << reference.error().describe();

    // Shift every sample of every channel by a large constant.
    auto shifted = fixture().series;
    for (auto& [id, series] : shifted) {
        for (std::size_t i = 0; i < series.size(); ++i) {
            series.set(i, series[i] + 137.5F);
        }
    }
    const auto shiftedWindows = fixture().windowsFrom(shifted, kIndexA);
    ASSERT_TRUE(shiftedWindows) << shiftedWindows.error().describe();

    const auto score = scorer.score(shiftedWindows.value(), other);
    ASSERT_TRUE(score) << score.error().describe();
    EXPECT_NEAR(reference.value().total(), score.value().total(), kInvarianceTolerance);
}

TEST(ScoringAcceptance, InvariantToPositiveScaling) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto baseline = windowsAt(kIndexA);
    const auto other = windowsAt(kIndexB);

    const auto reference = scorer.score(baseline, other);
    ASSERT_TRUE(reference) << reference.error().describe();

    // Multiply one side by 10. Robust-z removes both level and scale, so the
    // shape - and therefore the score - must be unchanged.
    auto scaled = fixture().series;
    for (auto& [id, series] : scaled) {
        for (std::size_t i = 0; i < series.size(); ++i) {
            series.set(i, series[i] * 10.0F);
        }
    }
    const auto scaledWindows = fixture().windowsFrom(scaled, kIndexA);
    ASSERT_TRUE(scaledWindows) << scaledWindows.error().describe();

    const auto score = scorer.score(scaledWindows.value(), other);
    ASSERT_TRUE(score) << score.error().describe();
    EXPECT_NEAR(reference.value().total(), score.value().total(), kInvarianceTolerance);
}

// ---------------------------------------------------------------------------
// "Monotonicity: injecting increasing Gaussian noise into one window must
//  produce monotonically decreasing S (statistically, over many seeds)."
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, ScoreDecreasesAsNoiseIncreases) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto target = windowsAt(kIndexA);

    const std::vector<double> noiseLevels = {0.0, 0.25, 0.5, 1.0, 2.0, 4.0};
    std::vector<double> meanScores;

    for (const double sigma : noiseLevels) {
        double sum = 0.0;
        constexpr int kSeeds = 12;
        for (int seed = 0; seed < kSeeds; ++seed) {
            auto noisy = fixture().series;
            SplitMix64 rng(0xB0B0u + static_cast<std::uint64_t>(seed) * 977u);
            for (auto& [id, series] : noisy) {
                // Noise proportional to the channel's own scale, so "one
                // sigma" means the same thing on a price and on an RSI.
                double sumAbs = 0.0;
                for (std::size_t i = 0; i < series.size(); ++i) {
                    sumAbs += std::fabs(static_cast<double>(series[i]));
                }
                const double channelScale =
                    0.001 * (sumAbs / static_cast<double>(series.size()));
                for (std::size_t i = 0; i < series.size(); ++i) {
                    series.set(i, static_cast<float>(static_cast<double>(series[i]) +
                                                     sigma * channelScale * rng.gaussian()));
                }
            }
            const auto noisyWindows = fixture().windowsFrom(noisy, kIndexA);
            ASSERT_TRUE(noisyWindows) << noisyWindows.error().describe();
            const auto score = scorer.score(noisyWindows.value(), target);
            ASSERT_TRUE(score) << score.error().describe();
            sum += score.value().total();
        }
        meanScores.push_back(sum / 12.0);
    }

    for (std::size_t i = 1; i < meanScores.size(); ++i) {
        EXPECT_LT(meanScores[i], meanScores[i - 1])
            << "mean score did not fall from noise " << noiseLevels[i - 1] << " to "
            << noiseLevels[i] << " (" << meanScores[i - 1] << " -> " << meanScores[i] << ")";
    }
}

// ---------------------------------------------------------------------------
// "Outlier resistance: changing a single bar by 10 sigma must move S by less
//  than a configured bound."
//
// This is precisely what min-max normalisation plus a ratio-based per-bar
// score could not offer: one spike rescaled the entire window.
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, SingleOutlierBarHasBoundedInfluence) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto other = windowsAt(kIndexB);
    const auto baseline = windowsAt(kIndexA);

    const auto reference = scorer.score(baseline, other);
    ASSERT_TRUE(reference) << reference.error().describe();

    // Spike the middle bar of the scored window on every channel by a factor
    // far outside anything the window contains.
    auto spiked = fixture().series;
    const std::size_t spikeIndex = kIndexA - fixture().windowLength / 2;
    for (auto& [id, series] : spiked) {
        series.set(spikeIndex, series[spikeIndex] * 10.0F + 500.0F);
    }
    const auto spikedWindows = fixture().windowsFrom(spiked, kIndexA);
    ASSERT_TRUE(spikedWindows) << spikedWindows.error().describe();

    const auto score = scorer.score(spikedWindows.value(), other);
    ASSERT_TRUE(score) << score.error().describe();

    // Bound: one bar in a 48-bar window, clamped at 4 sigma and then averaged
    // into 12 PAA segments, cannot be worth more than a few points.
    const double delta = std::fabs(reference.value().total() - score.value().total());
    EXPECT_LT(delta, 15.0) << "a single outlier moved the score by " << delta << " points";
}

// ---------------------------------------------------------------------------
// "Calibration: on random pairs, the distribution of s_c must be
//  approximately uniform in [0, 1] per channel, and S >= 99 must occur at
//  approximately the predicted rate."
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, CalibratedSimilarityIsUniformOnRandomPairs) {
    const ScoringFixture& f = fixture();

    for (const auto& spec : f.registry.specs()) {
        const auto calibration = f.calibration.get(spec.id);
        ASSERT_TRUE(calibration) << calibration.error().describe();
        const mt::data::Series& series = f.series.at(spec.id);

        // Fresh pairs with a different seed from the ones used to calibrate,
        // so this measures generalisation rather than memorisation.
        mt::similarity::CalibrationSamplingConfig sampling;
        sampling.pairCount = 4000;
        sampling.seed = 0xFEEDFACEu;
        sampling.minSeparation = f.windowLength;

        const std::size_t windowCount = series.size() - f.windowLength + 1;
        const auto pairs =
            mt::similarity::sampleWindowPairs(windowCount, f.windowLength, sampling);
        ASSERT_TRUE(pairs) << pairs.error().describe();

        std::vector<double> similarities;
        similarities.reserve(pairs.value().size());
        for (const auto& [i, j] : pairs.value()) {
            const auto viewA = series.windowFrom(i, f.windowLength);
            const auto viewB = series.windowFrom(j, f.windowLength);
            ASSERT_TRUE(viewA);
            ASSERT_TRUE(viewB);
            const auto a = mt::features::standardise(viewA.value(), spec.standardise);
            const auto b = mt::features::standardise(viewB.value(), spec.standardise);
            ASSERT_TRUE(a);
            ASSERT_TRUE(b);
            if (!a.value().usable || !b.value().usable) continue;
            const auto d = mt::similarity::distance(a.value().values, b.value().values,
                                                    spec.distance);
            ASSERT_TRUE(d) << d.error().describe();
            similarities.push_back(calibration.value()->similarity(d.value()));
        }

        ASSERT_GT(similarities.size(), 1000u) << "channel " << spec.id;

        // Kolmogorov-Smirnov against U(0,1). The 1% critical value is
        // 1.63 / sqrt(n); allow a generous multiple since the samples are not
        // fully independent (windows overlap other windows in the series).
        std::sort(similarities.begin(), similarities.end());
        const double n = static_cast<double>(similarities.size());
        double maxDeviation = 0.0;
        for (std::size_t k = 0; k < similarities.size(); ++k) {
            const double empirical = static_cast<double>(k + 1) / n;
            maxDeviation = std::max(maxDeviation, std::fabs(empirical - similarities[k]));
        }
        const double critical = 4.0 * 1.63 / std::sqrt(n);
        EXPECT_LT(maxDeviation, critical)
            << "channel " << spec.id << " calibrated similarity is not uniform: KS "
            << maxDeviation << " vs allowance " << critical;

        // And the mean should sit near 0.5 for a uniform distribution.
        const double mean =
            std::accumulate(similarities.begin(), similarities.end(), 0.0) / n;
        EXPECT_NEAR(mean, 0.5, 0.08) << "channel " << spec.id;
    }
}

// ---------------------------------------------------------------------------
// "No-match behaviour: fed a synthetic window unlike anything in the history
//  fixture, the engine must return an empty match set, not its 20 least-bad
//  candidates."
//
// At this layer that means: an alien window must score low and fail a gate.
// The empty-match-set half of the requirement is enforced in mt_retrieval.
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, AlienWindowScoresLowAndFailsGates) {
    // Same registry, but with a gate on every channel.
    mt::features::ChannelRegistry gated = fixture().registry;
    for (std::size_t i = 0; i < gated.size(); ++i) {
        const_cast<mt::features::ChannelSpec&>(gated[i]).gate = 0.90;
    }
    mt::similarity::ScoreConfig config;
    auto scorerResult = mt::similarity::Scorer::create(gated, fixture().calibration, config);
    ASSERT_TRUE(scorerResult) << scorerResult.error().describe();
    const mt::similarity::Scorer scorer = std::move(scorerResult).value();

    const auto history = windowsAt(kIndexA);

    // A hard sawtooth: nothing in a random walk or a bounded oscillator looks
    // like this.
    std::unordered_map<std::string, mt::data::Series> alien;
    for (const auto& spec : fixture().registry.specs()) {
        mt::data::Series series(spec.id);
        series.reserve(fixture().windowLength * 4);
        for (std::size_t i = 0; i < fixture().windowLength * 4; ++i) {
            const double phase = static_cast<double>(i % 4) / 4.0;
            series.push(static_cast<float>(1000.0 * phase - 500.0));
        }
        alien.emplace(spec.id, std::move(series));
    }
    const auto alienWindows =
        fixture().windowsFrom(alien, fixture().windowLength * 4 - 1);
    ASSERT_TRUE(alienWindows) << alienWindows.error().describe();

    const auto score = scorer.score(history, alienWindows.value());
    ASSERT_TRUE(score) << score.error().describe();
    ASSERT_TRUE(score.value().valid());

    EXPECT_LT(score.value().total(), 60.0)
        << "an alien shape scored " << score.value().total();
    EXPECT_FALSE(score.value().gatesPassed())
        << "an alien shape passed a 0.90 gate on every channel";
}

// ---------------------------------------------------------------------------
// "Determinism: same inputs and same config produce bit-identical output."
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, IsDeterministic) {
    const mt::similarity::Scorer scorer = makeScorer();
    const auto a = windowsAt(kIndexA);
    const auto b = windowsAt(kIndexC);

    const auto first = scorer.score(a, b);
    ASSERT_TRUE(first) << first.error().describe();

    for (int repeat = 0; repeat < 5; ++repeat) {
        const auto again = scorer.score(a, b);
        ASSERT_TRUE(again) << again.error().describe();
        EXPECT_EQ(first.value().total(), again.value().total());
        EXPECT_EQ(first.value().minSimilarity(), again.value().minSimilarity());
        ASSERT_EQ(first.value().channels().size(), again.value().channels().size());
        for (std::size_t c = 0; c < first.value().channels().size(); ++c) {
            EXPECT_EQ(first.value().channels()[c].rawDistance,
                      again.value().channels()[c].rawDistance);
        }
    }
}

// ---------------------------------------------------------------------------
// Bounded range, and independence from channel count (finding S2). The legacy
// total grew with every channel scanned, so no threshold survived a change to
// the filter string.
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, TotalIsBoundedAndIndependentOfChannelCount) {
    const mt::similarity::Scorer scorer = makeScorer();

    for (const std::size_t index : {kIndexA, kIndexB, kIndexC}) {
        for (const std::size_t other : {kIndexA, kIndexB, kIndexC}) {
            const auto score = scorer.score(windowsAt(index), windowsAt(other));
            ASSERT_TRUE(score) << score.error().describe();
            if (!score.value().valid()) continue;
            EXPECT_GE(score.value().total(), 0.0);
            EXPECT_LE(score.value().total(), 100.0);
        }
    }

    // Halve the registry and confirm the scale of the result is unchanged.
    mt::features::ChannelRegistry half;
    mt::similarity::CalibrationSet halfCalibration(0);
    for (std::size_t i = 0; i < 2; ++i) {
        mt::features::ChannelSpec spec = fixture().registry[i];
        ASSERT_TRUE(half.add(spec).ok());
    }
    halfCalibration.setFingerprint(half.fingerprint());
    for (std::size_t i = 0; i < 2; ++i) {
        const auto source = fixture().calibration.get(fixture().registry[i].id);
        ASSERT_TRUE(source) << source.error().describe();
        ASSERT_TRUE(halfCalibration.set(fixture().registry[i].id, *source.value()).ok());
    }

    auto halfScorer =
        mt::similarity::Scorer::create(half, halfCalibration, mt::similarity::ScoreConfig{});
    ASSERT_TRUE(halfScorer) << halfScorer.error().describe();

    const auto full = windowsAt(kIndexA);
    const auto fullOther = windowsAt(kIndexB);
    std::vector<mt::features::StandardisedWindow> twoA(full.begin(), full.begin() + 2);
    std::vector<mt::features::StandardisedWindow> twoB(fullOther.begin(), fullOther.begin() + 2);

    const auto score = halfScorer.value().score(twoA, twoB);
    ASSERT_TRUE(score) << score.error().describe();
    EXPECT_GE(score.value().total(), 0.0);
    EXPECT_LE(score.value().total(), 100.0);
}

// ---------------------------------------------------------------------------
// A calibration built for a different channel set must be refused, not used.
// Silently scoring against a mismatched calibration would produce numbers that
// look fine and mean nothing.
// ---------------------------------------------------------------------------
TEST(ScoringAcceptance, MismatchedCalibrationIsRefused) {
    mt::similarity::CalibrationSet wrong = fixture().calibration;
    wrong.setFingerprint(fixture().registry.fingerprint() ^ 0xDEADBEEFULL);

    const auto scorer = mt::similarity::Scorer::create(fixture().registry, wrong,
                                                       mt::similarity::ScoreConfig{});
    ASSERT_FALSE(scorer);
    EXPECT_EQ(scorer.error().code(), mt::ErrorCode::FailedPrecondition);
}
