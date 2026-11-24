// Tests for mt_base: explicit failure propagation, injectable logging and time.
#include <gtest/gtest.h>

#include <string>

#include "mt/base/clock.h"
#include "mt/base/error.h"
#include "mt/base/log.h"

namespace {

mt::Result<int> parsePositive(const std::string& text) {
    if (text.empty()) {
        return mt::invalidArgument("empty input");
    }
    const int value = std::stoi(text);
    if (value <= 0) {
        return mt::outOfRange("not positive: " + text);
    }
    return value;
}

}  // namespace

TEST(Base, ResultCarriesValue) {
    const auto result = parsePositive("42");
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

TEST(Base, ResultCarriesError) {
    const auto result = parsePositive("-1");
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), mt::ErrorCode::OutOfRange);
    EXPECT_NE(result.error().message().find("-1"), std::string::npos);
    EXPECT_EQ(result.error().describe(), "OutOfRange: not positive: -1");
}

TEST(Base, ValueOrAvoidsBranchingAtCallSites) {
    EXPECT_EQ(parsePositive("7").valueOr(-1), 7);
    EXPECT_EQ(parsePositive("").valueOr(-1), -1);
}

TEST(Base, StatusDistinguishesSuccessFromFailure) {
    const mt::Status ok = mt::Status::success();
    EXPECT_TRUE(ok.ok());

    const mt::Status bad = mt::Status(mt::dataCorrupt("bad row"));
    EXPECT_FALSE(bad.ok());
    EXPECT_EQ(bad.error().code(), mt::ErrorCode::DataCorrupt);
}

TEST(Base, ErrorCodeNamesAreStable) {
    // These names appear in logs and artefacts, so they are part of the
    // contract rather than an implementation detail.
    EXPECT_EQ(mt::toString(mt::ErrorCode::InvalidArgument), "InvalidArgument");
    EXPECT_EQ(mt::toString(mt::ErrorCode::FailedPrecondition), "FailedPrecondition");
    EXPECT_EQ(mt::toString(mt::ErrorCode::NotFound), "NotFound");
}

TEST(Base, CapturingLoggerRecordsWhatWasLogged) {
    mt::CapturingLogger logger;
    logger.info("starting");
    logger.warn("odd");
    logger.error("broken");

    EXPECT_EQ(logger.records().size(), 3u);
    EXPECT_EQ(logger.count(mt::LogLevel::Warn), 1u);
    EXPECT_EQ(logger.count(mt::LogLevel::Error), 1u);
    EXPECT_EQ(logger.count(mt::LogLevel::Debug), 0u);
    EXPECT_EQ(logger.records().front().message, "starting");
}

TEST(Base, NullLoggerDiscardsEverything) {
    // The default for pure computation paths: a component that takes an
    // ILogger& can be exercised without producing output.
    mt::ILogger& logger = mt::NullLogger::instance();
    logger.error("this goes nowhere");
    SUCCEED();
}

TEST(Base, ManualClockOnlyMovesWhenAdvanced) {
    mt::ManualClock clock(1'700'000'000'000);
    EXPECT_EQ(clock.nowMs(), 1'700'000'000'000);

    clock.advanceMs(60'000);
    EXPECT_EQ(clock.nowMs(), 1'700'000'060'000);

    // Repeated reads do not drift, which is what makes a backtest
    // reproducible.
    EXPECT_EQ(clock.nowMs(), clock.nowMs());
}

TEST(Base, ScopedDurationMeasuresAgainstTheInjectedClock) {
    mt::ManualClock clock(0);
    const mt::ScopedDuration duration(clock);
    EXPECT_EQ(duration.elapsedMs(), 0);

    clock.advanceMs(250);
    EXPECT_EQ(duration.elapsedMs(), 250);
}
