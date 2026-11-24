// ---------------------------------------------------------------------------
// End-to-end test of the whole new pipeline.
//
// Builds a synthetic history containing a *planted* motif that is reliably
// followed by an up-move, buried in noise. The engine must:
//
//   * find the earlier occurrences of the motif when shown a new one,
//   * aggregate them into an UP signal,
//   * decide to trade,
//   * and, when shown something unlike anything in history, produce no signal
//     at all rather than its least-bad candidates (finding S7).
//
// This is the test that proves the stages compose: channels, standardisation,
// calibration, embedding, ANN retrieval, exact rescoring, labelling,
// aggregation and policy, all in one chain on the same data.
// ---------------------------------------------------------------------------
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "mt/engine/engine.h"
#include "mt/similarity/calibrate.h"
#include "scoring_fixture.h"

namespace {

using mtbot::testing::SplitMix64;

constexpr std::size_t kWindowLength = 32;
constexpr std::size_t kHorizon = 10;
constexpr double kPointSize = 0.01;

// A distinctive shape: a sharp dip then a recovery, scaled to the local price.
// Nothing in a random walk reproduces this reliably, which is what makes it
// findable.
std::vector<double> motifShape() {
    return {0.0,  -0.4, -0.9, -1.5, -2.2, -2.6, -2.8, -2.6,
            -2.2, -1.6, -1.0, -0.5, -0.1, 0.2,  0.4,  0.5};
}

struct SyntheticHistory {
    mt::data::BarSeries bars;
    std::vector<std::size_t> motifEndIndices;  // where each planted motif ends
};

// Random walk with the motif planted every `spacing` bars, each occurrence
// followed by a consistent upward drift so the label is predictable.
SyntheticHistory makeHistory(std::size_t length, std::size_t spacing, std::uint64_t seed,
                             bool plantFollowThrough) {
    SplitMix64 rng(seed);
    SyntheticHistory history;
    history.bars = mt::data::BarSeries("SYNTH");
    history.bars.reserve(length);

    const std::vector<double> motif = motifShape();
    double level = 1800.0;
    std::size_t motifCursor = motif.size();  // not currently planting
    std::size_t followThrough = 0;

    for (std::size_t i = 0; i < length; ++i) {
        // Start a motif on schedule, with enough room for its follow-through.
        if (i % spacing == 0 && i > kWindowLength + 60 &&
            i + motif.size() + kHorizon + 5 < length) {
            motifCursor = 0;
        }

        double step;
        if (motifCursor < motif.size()) {
            // Planted shape: deterministic, plus a little noise so the
            // occurrences are similar rather than identical.
            const double target = 1800.0 + motif[motifCursor] * 1.2;
            step = (target - level) * 0.8 + rng.signedUnit() * 0.05;
            ++motifCursor;
            if (motifCursor == motif.size()) {
                history.motifEndIndices.push_back(i);
                followThrough = plantFollowThrough ? kHorizon : 0;
            }
        } else if (followThrough > 0) {
            // Consistent up-move after the motif: the thing the engine should
            // learn to expect.
            step = 0.55 + rng.signedUnit() * 0.08;
            --followThrough;
        } else {
            step = rng.signedUnit() * 0.35;
        }

        level += step;

        mt::data::Bar bar;
        bar.ts = static_cast<mt::Timestamp>(60'000 * (i + 1));
        bar.open = level - step;
        bar.close = level;
        const double wick = 0.12 + std::fabs(rng.signedUnit()) * 0.1;
        bar.high = std::max(bar.open, bar.close) + wick;
        bar.low = std::min(bar.open, bar.close) - wick;
        bar.volume = 1.0;
        if (!history.bars.append(bar).ok()) {
            ADD_FAILURE() << "synthetic bar rejected at index " << i;
        }
    }
    return history;
}

mt::engine::EngineConfig makeConfig() {
    mt::engine::EngineConfig config;
    config.windowLength = kWindowLength;

    config.label.horizonBars = kHorizon;
    config.label.touchPoints = 40.0;  // 0.40 in price, reachable by the drift
    config.label.pointSize = kPointSize;
    config.label.maxBarGapMs = 120'000;

    config.search.topM = 400;
    config.score.lambdaMin = 0.25;

    config.minScore = 55.0;
    config.rescoreTopK = 40;

    config.aggregate.priorStrength = 1.0;
    config.aggregate.wilsonZ = 1.645;

    config.policy.minProbabilityLowerBound = 0.50;
    config.policy.minEffectiveSampleSize = 4.0;
    config.policy.minMeanScore = 55.0;
    config.policy.maxNoiseRatio = 0.60;
    config.policy.deriveStopsFromMatches = true;
    config.policy.minStopPoints = 10.0;
    config.policy.maxStopPoints = 500.0;

    return config;
}

// Builds a fully wired engine over the given history.
mt::Result<std::unique_ptr<mt::engine::PatternEngine>> makeEngine(
    const mt::data::BarSeries& bars, mt::engine::EngineConfig config) {
    mt::features::ChannelRegistry registry =
        mt::features::defaultEngineRegistry(/*paaSegments=*/8);

    mt::Result<mt::features::ChannelSet> channels = mt::features::buildDefaultChannels(bars);
    if (!channels) {
        return channels.error();
    }

    mt::similarity::CalibrationBuildConfig calibrationConfig;
    calibrationConfig.windowLength = config.windowLength;
    calibrationConfig.sampling.pairCount = 8000;
    calibrationConfig.sampling.bins = 256;

    mt::similarity::CalibrationSet calibration;
    mt::Result<mt::similarity::CalibrationBuildReport> report =
        mt::similarity::buildCalibration(registry, channels.value(), calibrationConfig,
                                         calibration);
    if (!report) {
        return report.error();
    }

    return mt::engine::PatternEngine::build(bars, registry, std::move(calibration), config);
}

}  // namespace

TEST(EngineEndToEnd, BuildsOverSyntheticHistory) {
    const SyntheticHistory history = makeHistory(6000, 300, 0xE2E1, true);
    ASSERT_GT(history.motifEndIndices.size(), 10u) << "fixture planted too few motifs";

    const auto engine = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engine) << engine.error().describe();
    EXPECT_GT(engine.value()->embeddingRows(), 5000u);
    EXPECT_GT(engine.value()->firstEvaluableIndex(), kWindowLength);
}

TEST(EngineEndToEnd, FindsThePlantedMotifAndPredictsItsFollowThrough) {
    const SyntheticHistory history = makeHistory(6000, 300, 0xE2E1, true);
    const auto engineResult = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engineResult) << engineResult.error().describe();
    const mt::engine::PatternEngine& engine = *engineResult.value();

    // Evaluate at the end of a late motif occurrence: all the earlier ones are
    // outside the embargo and should be retrievable.
    const std::size_t target = history.motifEndIndices.back();
    ASSERT_GT(target, engine.firstEvaluableIndex());

    const auto result = engine.evaluateAt(target);
    ASSERT_TRUE(result) << result.error().describe();

    EXPECT_GT(result.value().diagnostics.indexCandidates, 0u);
    EXPECT_GT(result.value().diagnostics.aboveMinScore, 0u)
        << "no candidate cleared the quality cut, so the motif was not recognised";
    ASSERT_TRUE(result.value().signal.valid);

    // The planted follow-through is up, so the aggregated direction should be.
    EXPECT_EQ(result.value().signal.direction, mt::labels::Direction::Up)
        << "probability " << result.value().signal.probability << ", ESS "
        << result.value().signal.effectiveSampleSize;
    EXPECT_GT(result.value().signal.probability, 0.6);
    EXPECT_GT(result.value().signal.effectiveSampleSize, 3.0);

    // And the best match should be explainable: every channel accounted for.
    EXPECT_EQ(result.value().bestScore.channels().size(), engine.registry().size());
    EXPECT_GT(result.value().bestScore.total(), 55.0);
}

TEST(EngineEndToEnd, DecidesToTradeOnTheMotif) {
    const SyntheticHistory history = makeHistory(6000, 300, 0xE2E1, true);
    const auto engineResult = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    const std::size_t target = history.motifEndIndices.back();
    const auto result = engineResult.value()->evaluateAt(target);
    ASSERT_TRUE(result) << result.error().describe();

    EXPECT_TRUE(result.value().decision.trade)
        << "rejected because: " << mt::policy::toString(result.value().decision.reason)
        << " (p_lower " << result.value().signal.probabilityLowerBound << ", ESS "
        << result.value().signal.effectiveSampleSize << ", meanScore "
        << result.value().signal.meanScore << ")";
    EXPECT_EQ(result.value().decision.direction, mt::labels::Direction::Up);
    EXPECT_GT(result.value().decision.expectedEdgePoints, 0.0);
    EXPECT_GT(result.value().decision.lots, 0.0);
}

TEST(EngineEndToEnd, WithoutAFollowThroughThereIsNoEdgeToFind) {
    // Same motif, but what follows it is pure noise. The engine should still
    // *find* the matches and should not invent a direction from them.
    const SyntheticHistory history = makeHistory(6000, 300, 0xE2E1, false);
    const auto engineResult = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    const std::size_t target = history.motifEndIndices.back();
    const auto result = engineResult.value()->evaluateAt(target);
    ASSERT_TRUE(result) << result.error().describe();

    if (result.value().signal.valid) {
        // No consistent future means either a weak probability or a high noise
        // ratio - and in neither case a confident trade.
        const bool weak = result.value().signal.probabilityLowerBound < 0.60;
        const bool noisy = result.value().signal.noiseRatio > 0.3;
        EXPECT_TRUE(weak || noisy)
            << "an edge appeared where none was planted: p_lower "
            << result.value().signal.probabilityLowerBound << ", noise "
            << result.value().signal.noiseRatio;
    }
}

TEST(EngineEndToEnd, StandsAsideDuringWarmUp) {
    const SyntheticHistory history = makeHistory(3000, 300, 0xE2E2, true);
    const auto engineResult = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    // Before the window plus the embargo can fit, there is nothing to compare
    // against. That is a no-signal, not an error and not a guess.
    const auto result = engineResult.value()->evaluateAt(kWindowLength);
    ASSERT_TRUE(result) << result.error().describe();
    EXPECT_FALSE(result.value().signal.valid);
    EXPECT_FALSE(result.value().decision.trade);
    EXPECT_EQ(result.value().decision.reason, mt::policy::RejectReason::NoSignal);
}

TEST(EngineEndToEnd, ReturnsNoSignalWhenTheQualityBarIsUnreachable) {
    // Finding S7 at the pipeline level: with an impossible quality bar the
    // engine must return nothing, where the legacy top-fraction cascade would
    // still have handed back its twenty least-bad candidates.
    const SyntheticHistory history = makeHistory(4000, 300, 0xE2E3, true);
    mt::engine::EngineConfig config = makeConfig();
    config.minScore = 99.999;

    const auto engineResult = makeEngine(history.bars, config);
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    const std::size_t target = history.motifEndIndices.back();
    const auto result = engineResult.value()->evaluateAt(target);
    ASSERT_TRUE(result) << result.error().describe();

    EXPECT_EQ(result.value().diagnostics.aboveMinScore, 0u);
    EXPECT_FALSE(result.value().signal.valid);
    EXPECT_FALSE(result.value().decision.trade);
}

TEST(EngineEndToEnd, NeverReturnsACandidateInsideTheEmbargo) {
    const SyntheticHistory history = makeHistory(5000, 300, 0xE2E4, true);
    mt::engine::EngineConfig config = makeConfig();
    const auto engineResult = makeEngine(history.bars, config);
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    const std::size_t embargo = mt::labels::embargoBars(config.label, config.windowLength);

    for (const std::size_t target : {2000u, 3000u, 4000u}) {
        const auto result = engineResult.value()->evaluateAt(target);
        ASSERT_TRUE(result) << result.error().describe();
        for (const auto& match : result.value().matches) {
            EXPECT_LE(match.endIndex + embargo, target)
                << "match ending at " << match.endIndex << " is inside the embargo of "
                << target;
        }
    }
}

TEST(EngineEndToEnd, IsDeterministic) {
    const SyntheticHistory history = makeHistory(4000, 300, 0xE2E5, true);
    const auto engineResult = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    const std::size_t target = history.motifEndIndices.back();
    const auto first = engineResult.value()->evaluateAt(target);
    ASSERT_TRUE(first) << first.error().describe();

    for (int repeat = 0; repeat < 3; ++repeat) {
        const auto again = engineResult.value()->evaluateAt(target);
        ASSERT_TRUE(again);
        EXPECT_EQ(first.value().signal.direction, again.value().signal.direction);
        EXPECT_DOUBLE_EQ(first.value().signal.probability, again.value().signal.probability);
        EXPECT_EQ(first.value().matches.size(), again.value().matches.size());
        EXPECT_EQ(first.value().decision.trade, again.value().decision.trade);
    }
}

TEST(EngineEndToEnd, ApproximateAndExactIndexesAgreeOnTheDecision) {
    // The ANN index is an optimisation, not a change of behaviour. The recall
    // gate in test_retrieval.cpp bounds how often they can differ; here the
    // requirement is that they reach the same conclusion on the planted motif.
    const SyntheticHistory history = makeHistory(5000, 300, 0xE2E6, true);

    mt::engine::EngineConfig approximate = makeConfig();
    approximate.useApproximateIndex = true;
    mt::engine::EngineConfig exact = makeConfig();
    exact.useApproximateIndex = false;

    const auto annEngine = makeEngine(history.bars, approximate);
    const auto exactEngine = makeEngine(history.bars, exact);
    ASSERT_TRUE(annEngine) << annEngine.error().describe();
    ASSERT_TRUE(exactEngine) << exactEngine.error().describe();

    const std::size_t target = history.motifEndIndices.back();
    const auto annResult = annEngine.value()->evaluateAt(target);
    const auto exactResult = exactEngine.value()->evaluateAt(target);
    ASSERT_TRUE(annResult) << annResult.error().describe();
    ASSERT_TRUE(exactResult) << exactResult.error().describe();

    EXPECT_EQ(annResult.value().signal.direction, exactResult.value().signal.direction);
    EXPECT_EQ(annResult.value().decision.trade, exactResult.value().decision.trade);
    EXPECT_NEAR(annResult.value().signal.probability, exactResult.value().signal.probability,
                0.15);
}

TEST(EngineEndToEnd, DiagnosticsNarrowMonotonicallyThroughTheFunnel) {
    const SyntheticHistory history = makeHistory(5000, 300, 0xE2E7, true);
    const auto engineResult = makeEngine(history.bars, makeConfig());
    ASSERT_TRUE(engineResult) << engineResult.error().describe();

    const std::size_t target = history.motifEndIndices.back();
    const auto result = engineResult.value()->evaluateAt(target);
    ASSERT_TRUE(result) << result.error().describe();

    const auto& d = result.value().diagnostics;
    EXPECT_LE(d.afterFilters, d.indexCandidates);
    EXPECT_LE(d.rescored, d.afterFilters);
    EXPECT_LE(d.aboveMinScore, d.rescored);
    EXPECT_LE(d.labelled, d.aboveMinScore);
    EXPECT_LE(d.afterDedup, d.labelled);
}
