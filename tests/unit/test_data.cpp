// Tests for mt_data: validity masks instead of sentinels (finding B6),
// value semantics, and window arithmetic that fails loudly at the edges.
#include <gtest/gtest.h>

#include "mt/data/bar.h"
#include "mt/data/series.h"

namespace {

mt::data::Series makeRamp(std::size_t n) {
    mt::data::Series series("ramp");
    for (std::size_t i = 0; i < n; ++i) {
        series.push(static_cast<float>(i));
    }
    return series;
}

}  // namespace

TEST(Series, MissingIsABitNotASentinel) {
    // The legacy code wrote -1.0f for a missing indicator, which then fed the
    // rolling min/max and the similarity comparison (finding B6). Here the
    // absence is recorded separately and no numeric value can be mistaken for
    // data.
    mt::data::Series series("ema");
    series.push(1.0F);
    series.pushMissing();
    series.push(3.0F);

    EXPECT_EQ(series.size(), 3u);
    EXPECT_TRUE(series.isValid(0));
    EXPECT_FALSE(series.isValid(1));
    EXPECT_TRUE(series.isValid(2));
    EXPECT_EQ(series.validCount(), 2u);

    // -1.0f is a perfectly ordinary value here, not a signal.
    mt::data::Series withNegative("slope");
    withNegative.push(-1.0F);
    EXPECT_TRUE(withNegative.isValid(0));
    EXPECT_FLOAT_EQ(withNegative[0], -1.0F);
}

TEST(Series, WindowEndingAtIndexCoversTheExpectedSamples) {
    const mt::data::Series series = makeRamp(100);

    const auto window = series.window(49, 10);
    ASSERT_TRUE(window) << window.error().describe();
    ASSERT_EQ(window.value().size(), 10u);
    EXPECT_FLOAT_EQ(window.value()[0], 40.0F);
    EXPECT_FLOAT_EQ(window.value()[9], 49.0F);
}

TEST(Series, WindowRefusesToSilentlyShorten) {
    const mt::data::Series series = makeRamp(100);

    // Not enough history before index 3 for a 10-sample window. Clamping here
    // is how comparisons end up being made between different numbers of bars.
    const auto tooEarly = series.window(3, 10);
    ASSERT_FALSE(tooEarly);
    EXPECT_EQ(tooEarly.error().code(), mt::ErrorCode::OutOfRange);

    const auto pastEnd = series.window(100, 10);
    ASSERT_FALSE(pastEnd);
    EXPECT_EQ(pastEnd.error().code(), mt::ErrorCode::OutOfRange);

    const auto zeroLength = series.window(50, 0);
    ASSERT_FALSE(zeroLength);
    EXPECT_EQ(zeroLength.error().code(), mt::ErrorCode::InvalidArgument);
}

TEST(Series, WindowFromBeginIndexMatchesTheScanEnumeration) {
    const mt::data::Series series = makeRamp(100);

    const auto window = series.windowFrom(0, 10);
    ASSERT_TRUE(window);
    EXPECT_FLOAT_EQ(window.value()[0], 0.0F);
    EXPECT_FLOAT_EQ(window.value()[9], 9.0F);

    // The last window that fits.
    const auto last = series.windowFrom(90, 10);
    ASSERT_TRUE(last);
    EXPECT_FLOAT_EQ(last.value()[9], 99.0F);

    ASSERT_FALSE(series.windowFrom(91, 10));
}

TEST(Series, WindowCountIsTheCandidateCount) {
    const mt::data::Series series = makeRamp(100);
    // The legacy code spelled this as histSize - patternLen + 1 in a dozen
    // places; here it is one function with the edge cases covered.
    EXPECT_EQ(series.windowCount(10), 91u);
    EXPECT_EQ(series.windowCount(100), 1u);
    EXPECT_EQ(series.windowCount(101), 0u);
    EXPECT_EQ(series.windowCount(0), 0u);
}

TEST(WindowView, ReportsValidityRatio) {
    mt::data::Series series("mixed");
    for (int i = 0; i < 10; ++i) {
        if (i == 3 || i == 7) {
            series.pushMissing();
        } else {
            series.push(static_cast<float>(i));
        }
    }

    const auto window = series.window(9, 10);
    ASSERT_TRUE(window);
    EXPECT_EQ(window.value().validCount(), 8u);
    EXPECT_DOUBLE_EQ(window.value().validRatio(), 0.8);
    EXPECT_FALSE(window.value().allValid());
}

TEST(BarSeries, RejectsOutOfOrderTimestamps) {
    mt::data::BarSeries series("XAUUSD");
    ASSERT_TRUE(series.append({1000, 10.0, 11.0, 9.0, 10.5, 1.0}).ok());

    // Equal or earlier timestamps break every rolling computation downstream,
    // so they are refused at the boundary rather than tolerated.
    const mt::Status duplicate = series.append({1000, 10.0, 11.0, 9.0, 10.5, 1.0});
    EXPECT_FALSE(duplicate.ok());
    EXPECT_EQ(duplicate.error().code(), mt::ErrorCode::DataCorrupt);

    const mt::Status backwards = series.append({900, 10.0, 11.0, 9.0, 10.5, 1.0});
    EXPECT_FALSE(backwards.ok());
}

TEST(BarSeries, RejectsIncoherentOhlc) {
    mt::data::BarSeries series("XAUUSD");
    // high below close: a mid-bar snapshot or an interleaved feed.
    const mt::Status status = series.append({1000, 10.0, 10.2, 9.0, 10.5, 1.0});
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.error().code(), mt::ErrorCode::DataCorrupt);
}

TEST(BarSeries, FindsIndexByTimestamp) {
    mt::data::BarSeries series("XAUUSD");
    for (int i = 0; i < 10; ++i) {
        const mt::Timestamp ts = 60'000 * (i + 1);
        ASSERT_TRUE(series.append({ts, 10.0, 11.0, 9.0, 10.5, 1.0}).ok());
    }

    const auto exact = series.indexOf(300'000);
    ASSERT_TRUE(exact);
    EXPECT_EQ(exact.value(), 4u);

    ASSERT_FALSE(series.indexOf(301'000));

    // At-or-before is what a backtest asks when stepping simulated time.
    const auto before = series.indexAtOrBefore(301'000);
    ASSERT_TRUE(before);
    EXPECT_EQ(before.value(), 4u);

    ASSERT_FALSE(series.indexAtOrBefore(1'000));
}

TEST(BarSeries, FindsGapsThatPatternsMustNotSpan) {
    mt::data::BarSeries series("XAUUSD");
    const mt::Timestamp minute = 60'000;
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(series.append({minute * (i + 1), 10.0, 11.0, 9.0, 10.5, 1.0}).ok());
    }
    // A weekend-sized hole.
    ASSERT_TRUE(series.append({minute * 5000, 10.0, 11.0, 9.0, 10.5, 1.0}).ok());
    for (int i = 1; i < 4; ++i) {
        ASSERT_TRUE(series.append({minute * (5000 + i), 10.0, 11.0, 9.0, 10.5, 1.0}).ok());
    }

    const auto gaps = series.findGaps(minute);
    ASSERT_EQ(gaps.size(), 1u);
    EXPECT_EQ(gaps.front(), 5u);
}

TEST(BarSeries, ValidatesTheWholeSeriesInOnePass) {
    mt::data::BarSeries series("XAUUSD");
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(series.append({60'000 * (i + 1), 10.0, 11.0, 9.0, 10.5, 1.0}).ok());
    }
    EXPECT_TRUE(series.validate().ok());
}
