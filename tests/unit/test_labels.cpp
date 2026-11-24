// Tests for mt_labels: the first-touch definition that fixes finding S9, and
// the leakage rules that fix B12.
#include <gtest/gtest.h>

#include <array>
#include <vector>

#include "mt/labels/label.h"

namespace {

// Builds a series where each bar's high/low can be set independently of the
// close, which is exactly what the legacy Pip_Change summation could not see.
mt::data::BarSeries makeBars(const std::vector<std::array<double, 4>>& ohlc,
                             mt::Timestamp spacingMs = 60'000) {
    mt::data::BarSeries series("TEST");
    mt::Timestamp ts = spacingMs;
    for (const auto& bar : ohlc) {
        mt::data::Bar b;
        b.ts = ts;
        b.open = bar[0];
        b.high = bar[1];
        b.low = bar[2];
        b.close = bar[3];
        if (!series.append(b).ok()) {
            ADD_FAILURE() << "fixture bar rejected at ts " << ts;
        }
        ts += spacingMs;
    }
    return series;
}

mt::labels::LabelConfig config(double touchPoints = 10.0, std::size_t horizon = 5) {
    mt::labels::LabelConfig c;
    c.touchPoints = touchPoints;
    c.pointSize = 1.0;  // one point == one price unit, so the tests read plainly
    c.horizonBars = horizon;
    return c;
}

}  // namespace

TEST(Labels, FirstTouchUsesIntrabarHighNotJustTheClose) {
    // The high of bar 2 reaches the up barrier while every close stays below
    // it. The legacy label summed per-bar close changes and would have missed
    // this entirely (finding S9).
    const auto bars = makeBars({
        {100, 100, 100, 100},  // entry
        {100, 103, 99, 101},
        {101, 115, 100, 102},  // high 115 >= 110
        {102, 104, 101, 103},
        {103, 104, 102, 103},
        {103, 104, 102, 103},
    });

    const auto label = mt::labels::labelForward(bars, 0, config());
    ASSERT_TRUE(label) << label.error().describe();
    ASSERT_TRUE(label.value().valid);
    EXPECT_EQ(label.value().firstTouch, mt::labels::Direction::Up);
    EXPECT_EQ(label.value().barsToTouch, 2u);
}

TEST(Labels, FirstTouchUsesIntrabarLow) {
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 101, 99, 100},
        {100, 101, 88, 99},  // low 88 <= 90
        {99, 100, 98, 99},
        {99, 100, 98, 99},
        {99, 100, 98, 99},
    });

    const auto label = mt::labels::labelForward(bars, 0, config());
    ASSERT_TRUE(label);
    EXPECT_EQ(label.value().firstTouch, mt::labels::Direction::Down);
    EXPECT_EQ(label.value().barsToTouch, 2u);
}

TEST(Labels, WhicheverBarrierComesFirstWins) {
    // Down is touched on bar 1, up only on bar 3. Order matters, and this is
    // the whole point of a first-touch label.
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 101, 89, 95},  // down first
        {95, 99, 94, 98},
        {98, 120, 97, 115},  // up later
        {115, 116, 114, 115},
        {115, 116, 114, 115},
    });

    const auto label = mt::labels::labelForward(bars, 0, config());
    ASSERT_TRUE(label);
    EXPECT_EQ(label.value().firstTouch, mt::labels::Direction::Down);
    EXPECT_EQ(label.value().barsToTouch, 1u);
}

TEST(Labels, SameCandleAmbiguityIsNoiseByDefault) {
    // One bar's range covers both barriers. OHLC cannot say which came first,
    // and pretending to know is how a backtest acquires an edge that is not
    // there.
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 115, 85, 100},  // spans both +10 and -10
        {100, 101, 99, 100},
        {100, 101, 99, 100},
        {100, 101, 99, 100},
        {100, 101, 99, 100},
    });

    const auto label = mt::labels::labelForward(bars, 0, config());
    ASSERT_TRUE(label);
    EXPECT_TRUE(label.value().sameCandleAmbiguity);
    EXPECT_EQ(label.value().firstTouch, mt::labels::Direction::Noise);
}

TEST(Labels, SameCandlePolicyCanBeOverriddenForSensitivityAnalysis) {
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 115, 85, 100},
        {100, 101, 99, 100},
        {100, 101, 99, 100},
        {100, 101, 99, 100},
        {100, 101, 99, 100},
    });

    auto up = config();
    up.sameCandlePolicy = mt::labels::SameCandlePolicy::UpFirst;
    const auto upLabel = mt::labels::labelForward(bars, 0, up);
    ASSERT_TRUE(upLabel);
    EXPECT_EQ(upLabel.value().firstTouch, mt::labels::Direction::Up);
    EXPECT_TRUE(upLabel.value().sameCandleAmbiguity) << "ambiguity is still reported";

    auto down = config();
    down.sameCandlePolicy = mt::labels::SameCandlePolicy::DownFirst;
    const auto downLabel = mt::labels::labelForward(bars, 0, down);
    ASSERT_TRUE(downLabel);
    EXPECT_EQ(downLabel.value().firstTouch, mt::labels::Direction::Down);
}

TEST(Labels, NeitherBarrierReachedIsNoiseNotMissing) {
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 102, 99, 101},
        {101, 103, 100, 102},
        {102, 103, 101, 102},
        {102, 103, 101, 102},
        {102, 103, 101, 102},
    });

    const auto label = mt::labels::labelForward(bars, 0, config());
    ASSERT_TRUE(label);
    EXPECT_TRUE(label.value().valid) << "a small move is information, not a missing value";
    EXPECT_EQ(label.value().firstTouch, mt::labels::Direction::Noise);
    EXPECT_EQ(label.value().barsToTouch, 5u);
}

TEST(Labels, HorizonRunningPastTheEndIsInvalid) {
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 102, 99, 101},
    });

    const auto label = mt::labels::labelForward(bars, 0, config(10.0, 5));
    ASSERT_TRUE(label) << label.error().describe();
    // Not an error - this is the normal condition at the right edge of
    // history, and the reason the newest bars can never be training data.
    EXPECT_FALSE(label.value().valid);
}

TEST(Labels, HorizonSpanningADataGapIsInvalid) {
    mt::data::BarSeries series("TEST");
    const mt::Timestamp minute = 60'000;
    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE(series.append({minute * (i + 1), 100, 100, 100, 100, 0}).ok());
    }
    // A weekend-sized hole in the middle of the horizon.
    ASSERT_TRUE(series.append({minute * 5000, 100, 100, 100, 100, 0}).ok());
    for (int i = 1; i < 5; ++i) {
        ASSERT_TRUE(series.append({minute * (5000 + i), 100, 100, 100, 100, 0}).ok());
    }

    const auto label = mt::labels::labelForward(series, 0, config(10.0, 5));
    ASSERT_TRUE(label);
    EXPECT_FALSE(label.value().valid)
        << "a horizon that spans a weekend is not a horizon";
}

TEST(Labels, ReportsExcursionsAndNetMove) {
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 108, 96, 104},
        {104, 106, 102, 105},
        {105, 107, 103, 106},
        {106, 108, 104, 107},
        {107, 109, 105, 108},
    });

    const auto label = mt::labels::labelForward(bars, 0, config(20.0, 5));
    ASSERT_TRUE(label);
    ASSERT_TRUE(label.value().valid);
    EXPECT_DOUBLE_EQ(label.value().netReturnPoints, 8.0);     // 108 - 100
    EXPECT_DOUBLE_EQ(label.value().maxFavourablePoints, 9.0);  // highest high 109
    EXPECT_DOUBLE_EQ(label.value().maxAdversePoints, -4.0);    // lowest low 96
}

TEST(Labels, ForwardHorizonNeverOverlapsThePattern) {
    // Entry at index 3 means the horizon starts at index 4. Making bar 3
    // itself breach the barrier must not produce a touch.
    const auto bars = makeBars({
        {100, 100, 100, 100},
        {100, 100, 100, 100},
        {100, 100, 100, 100},
        {100, 200, 100, 100},  // entry bar: huge high, must be ignored
        {100, 101, 99, 100},
        {100, 101, 99, 100},
        {100, 101, 99, 100},
    });

    const auto label = mt::labels::labelForward(bars, 3, config(10.0, 3));
    ASSERT_TRUE(label);
    ASSERT_TRUE(label.value().valid);
    EXPECT_EQ(label.value().firstTouch, mt::labels::Direction::Noise);
}

TEST(Labels, EmbargoCoversBothTheWindowAndTheHorizon) {
    const auto c = config(10.0, 15);
    constexpr std::size_t windowLength = 48;

    // A candidate must be far enough back that neither its own data nor its
    // forward horizon reaches into the present (finding B12).
    EXPECT_EQ(mt::labels::embargoBars(c, windowLength), 63u);

    EXPECT_TRUE(mt::labels::isCandidateAllowed(100, 163, c, windowLength));
    EXPECT_TRUE(mt::labels::isCandidateAllowed(100, 200, c, windowLength));
    EXPECT_FALSE(mt::labels::isCandidateAllowed(100, 162, c, windowLength));
    EXPECT_FALSE(mt::labels::isCandidateAllowed(100, 100, c, windowLength))
        << "a candidate ending at the present overlaps it entirely";
}

TEST(Labels, RejectsNonsenseConfiguration) {
    const auto bars = makeBars({{100, 100, 100, 100}, {100, 101, 99, 100}});

    auto zeroHorizon = config();
    zeroHorizon.horizonBars = 0;
    EXPECT_FALSE(mt::labels::labelForward(bars, 0, zeroHorizon));

    auto zeroBarrier = config();
    zeroBarrier.touchPoints = 0.0;
    EXPECT_FALSE(mt::labels::labelForward(bars, 0, zeroBarrier));

    EXPECT_FALSE(mt::labels::labelForward(bars, 99, config()));
}
