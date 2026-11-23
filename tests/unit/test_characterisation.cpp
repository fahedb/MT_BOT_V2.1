// ---------------------------------------------------------------------------
// Characterisation tests: executable evidence for the findings in
// docs/REFACTOR_PLAN.md section 4.
//
// These assert what the code does TODAY, including where that is wrong. They
// are not aspirational. Each one names the finding it demonstrates, and each
// is expected to be rewritten (not deleted) in the phase that fixes the
// finding — at which point the assertion flips and the plan's claim is
// settled by a test rather than by argument.
//
// The acceptance tests for the NEW scoring system live in section 6.9 of the
// plan and arrive in Phase 3.
// ---------------------------------------------------------------------------
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "baseline_compute.h"

namespace {

const mtbot::testing::BaselineInputs& inputs() {
    static const mtbot::testing::BaselineInputs in = mtbot::testing::makeBaselineInputs();
    return in;
}

float sum(const std::vector<float>& v) {
    float total = 0.0f;
    for (const float x : v) total += x;
    return total;
}

}  // namespace

// ---------------------------------------------------------------------------
// B5 — Compare_ByEval and Compare_ByRMSE are the same function.
//      1/(1+sqrt(mse)) vs 1/(1+rmse), and rmse == sqrt(mse).
//      PolynomComparer.cpp:58 and :80.
// ---------------------------------------------------------------------------
TEST(Finding_B5, EvalAndRmseAreTheSameMetric) {
    for (const auto& [pa, pb] : mtbot::testing::polynomProbePairs()) {
        const double byEval = PolynomComparer::Compare_ByEval(pa, pb);
        const double byRmse = PolynomComparer::Compare_ByRMSE(pa, pb);
        EXPECT_NEAR(byEval, byRmse, 1e-12)
            << "two of the three polynomial weights tune one metric";
    }
}

// ---------------------------------------------------------------------------
// S1 — RelativeScoreCalc is scale-relative, so a difference of 1e-8 scores
//      exactly like a difference of 100 as long as the ratio matches.
//      KernelEngine_ScoreCalcs_General.h:32.
// ---------------------------------------------------------------------------
TEST(Finding_S1, ScoreDependsOnRatioNotOnMagnitude) {
    const float tiny = RelativeScoreCalc(1e-8f, 2e-8f);
    const float large = RelativeScoreCalc(100.0f, 200.0f);
    EXPECT_FLOAT_EQ(tiny, large);
    EXPECT_FLOAT_EQ(tiny, 0.5f) << "noise at 1e-8 is scored as a half match";
}

TEST(Finding_S1, NearZeroValuesProduceStrongPartialMatches) {
    // Two values that are both effectively zero score 0.5, i.e. "half similar",
    // purely because their ratio is 2. Channels that sit near zero therefore
    // dominate the summed score.
    EXPECT_FLOAT_EQ(RelativeScoreCalc(0.001f, 0.002f), 0.5f);
    EXPECT_FLOAT_EQ(RelativeScoreCalc(0.0001f, 0.0002f), 0.5f);
}

TEST(Finding_S1, EqualMagnitudeSignFlipCollapsesToZero) {
    // +1 vs -1 is maximally dissimilar under this metric, the same as 1 vs 0,
    // so the metric cannot distinguish "opposite" from "absent".
    EXPECT_FLOAT_EQ(RelativeScoreCalc(-1.0f, 1.0f), 0.0f);
    EXPECT_FLOAT_EQ(RelativeScoreCalc(1.0f, 0.0f), 0.0f);
}

TEST(Finding_S1, IsNotATrueMetric_TriangleInequalityFails) {
    // Treating d = 1 - s as a distance: d(a,c) must be <= d(a,b) + d(b,c).
    const float a = 1.0f;
    const float b = 0.001f;
    const float c = -1.0f;
    const float dab = 1.0f - RelativeScoreCalc(a, b);
    const float dbc = 1.0f - RelativeScoreCalc(b, c);
    const float dac = 1.0f - RelativeScoreCalc(a, c);
    // Documented as failing or marginal; recorded so the new metric can assert
    // the opposite in Phase 3.
    EXPECT_GT(dac, 0.99f);
    EXPECT_GT(dab + dbc, 1.9f);
}

// ---------------------------------------------------------------------------
// S5 — Values are normalised per window, slopes are not. The consequence is
//      an asymmetry under a REGIME CHANGE, i.e. when the current window has a
//      different scale from history:
//        * value channels absorb it (min-max maps each window onto [0,1]),
//        * slope channels do not (they are fed raw into RelativeScoreCalc).
//
//      Note for future readers: scaling BOTH sides by the same factor changes
//      nothing, because RelativeScoreCalc is itself ratio-based
//      (1 - |h-c| / max(|h|,|c|)). Only a one-sided change exposes this.
//      KernelEngine_ScoreCalcs_MA.cu:4 / CPUEngine_ScoreCalcs_MA.cpp.
// ---------------------------------------------------------------------------
namespace {

void scaleBy(std::vector<float>& v, float factor) {
    for (float& x : v) x *= factor;
}

}  // namespace

TEST(Finding_S5, ValueChannelsAbsorbARegimeChangeButSlopeChannelsDoNot) {
    using namespace mtbot::testing;
    const BaselineInputs& base = inputs();
    const std::vector<float> reference = computeMaFullScan(base);

    // (a) the CURRENT window's values live at a 10x different scale from
    //     history. Per-window min-max normalisation cancels it exactly, so the
    //     score does not move at all.
    {
        BaselineInputs scaled = base;
        scaleBy(scaled.currMa.values, 10.0f);
        scaleBy(scaled.currMa.minValues, 10.0f);
        scaleBy(scaled.currMa.maxValues, 10.0f);

        const std::vector<float> scaledScores = computeMaFullScan(scaled);
        ASSERT_EQ(reference.size(), scaledScores.size());
        for (std::size_t i = 0; i < reference.size(); ++i) {
            const float scale = std::max(1.0f, std::fabs(reference[i]));
            EXPECT_LE(std::fabs(reference[i] - scaledScores[i]), 1e-3f * scale)
                << "value channels should be regime-invariant, at index " << i;
        }
    }

    // (b) the CURRENT window's slopes live at a 10x different scale. Nothing
    //     normalises them, so every slope comparison degrades and the total
    //     score drops materially. A volatile present therefore cannot be
    //     matched against a calm history, which is precisely what the engine
    //     is asked to do.
    {
        BaselineInputs scaled = base;
        scaleBy(scaled.currMa.slopes, 10.0f);

        const std::vector<float> scaledScores = computeMaFullScan(scaled);
        const float before = sum(reference);
        const float after = sum(scaledScores);

        EXPECT_NE(before, after) << "slope channels are not scale-normalised";
        EXPECT_LT(after, before * 0.95f)
            << "a 10x slope regime change costs real score: " << before << " -> " << after;
    }
}

// ---------------------------------------------------------------------------
// S2 — The total score is an unbounded sum: it scales with the number of
//      channels and with the window length, so a raw score threshold has no
//      stable meaning. BaseScoringEngine.cpp:6 and section 3.3.
// ---------------------------------------------------------------------------
TEST(Finding_S2, TotalScoreScalesWithChannelCount) {
    using namespace mtbot::testing;
    const BaselineInputs& in = inputs();

    // One channel.
    std::vector<float> one(static_cast<std::size_t>(kOutLen), 0.0f);
    FullScan_CPU_RSI(in.histRsi.values.data(), in.histRsi.slopes.data(),
                     in.currRsi.values.data(), in.currRsi.slopes.data(),
                     one.data(), 0, kOutLen, kPatternLen, kOutLen, kCompareWeight);

    // The engine accumulates channels into the same array with +=, so three
    // passes stand in for three channels.
    std::vector<float> three(static_cast<std::size_t>(kOutLen), 0.0f);
    for (int pass = 0; pass < 3; ++pass) {
        FullScan_CPU_RSI(in.histRsi.values.data(), in.histRsi.slopes.data(),
                         in.currRsi.values.data(), in.currRsi.slopes.data(),
                         three.data(), 0, kOutLen, kPatternLen, kOutLen, kCompareWeight);
    }

    EXPECT_NEAR(sum(three), 3.0f * sum(one), std::fabs(sum(one)) * 1e-4f)
        << "score magnitude is proportional to how many channels were scanned";

    // With the live default of 180 channels and a 50-bar window the perfect
    // match would score 10 * 2 * 50 * 180 = 180000, and nothing divides it back
    // down. See REFACTOR_PLAN section 3.3.
    EXPECT_GT(sum(three), sum(one));
}

// ---------------------------------------------------------------------------
// B7 — Find_MaxMinValues initialises min=1e6 and max=0, so an all-negative
//      series (every slope channel in a downtrend) reports max = 0.
//      KernelEngine_ScoreCalcs_General.h:12.
// ---------------------------------------------------------------------------
TEST(Finding_B7, FindMaxMinValuesIsWrongForAllNegativeSeries) {
    const float negatives[5] = {-5.0f, -3.0f, -9.0f, -1.5f, -7.0f};
    float result[2] = {0.0f, 0.0f};
    Find_MaxMinValues(negatives, 5, result);

    EXPECT_FLOAT_EQ(result[0], -9.0f) << "min is found correctly";
    EXPECT_FLOAT_EQ(result[1], 0.0f)
        << "max returns the initial 0.0f instead of -1.5f, because max is "
           "seeded with 0 rather than with the first element";
}

TEST(Finding_B7, FindMaxMinValuesIsWrongForLargeValues) {
    const float large[3] = {2.0e6f, 3.0e6f, 4.0e6f};
    float result[2] = {0.0f, 0.0f};
    Find_MaxMinValues(large, 3, result);

    EXPECT_FLOAT_EQ(result[0], 1.0e6f)
        << "min returns the initial 1e6 sentinel, which is below every input";
    EXPECT_FLOAT_EQ(result[1], 4.0e6f);
}

// ---------------------------------------------------------------------------
// Not yet testable at this layer (needs Phase 1 to break the singletons):
//   B1  cascade stages zero the score array (KernelExecutionContext)
//   B2  int truncation of the final score (PatternGod_ScoreComparer)
//   B4  unsorted threshold filter (PatternGodUtils + PatternGod)
//   B6  the -1.0f sentinel for missing indicators (TimeLineArrayBuilder)
//   S7  top-fraction cuts instead of quality cuts (BaseScoringEngine)
// Each is listed in the plan with its file:line and the phase that fixes it.
// ---------------------------------------------------------------------------
