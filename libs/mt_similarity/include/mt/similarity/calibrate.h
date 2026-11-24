// ---------------------------------------------------------------------------
// Building a calibration from real data — docs/REFACTOR_PLAN.md section 6.4.
//
// Samples random non-overlapping window pairs, measures each channel's
// distance over them, and turns the result into per-channel empirical CDFs.
//
// This is deliberately one implementation shared by apps/mtcalibrate and the
// tests. A calibration built differently from the one in production would make
// every test result meaningless.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <functional>

#include "mt/base/error.h"
#include "mt/base/log.h"
#include "mt/features/channel_set.h"
#include "mt/similarity/calibration.h"

namespace mt::similarity {

struct CalibrationBuildConfig {
    std::size_t windowLength = 48;
    CalibrationSamplingConfig sampling;

    // Pairs whose windows cannot be standardised (warm-up, data gaps) are
    // skipped. If more than this fraction is skipped the data is too sparse
    // for a trustworthy calibration and the build fails rather than producing
    // a CDF from whatever survived.
    double maxSkipRatio = 0.5;
};

struct CalibrationBuildReport {
    std::size_t pairsRequested = 0;
    std::size_t pairsUsable = 0;
    std::size_t channelsCalibrated = 0;
};

// `progress` is optional and is called with (channelIndex, channelCount) as
// each channel finishes, so a CLI can show something during a long run.
[[nodiscard]] Result<CalibrationBuildReport> buildCalibration(
    const features::ChannelRegistry& registry, const features::ChannelSet& channels,
    const CalibrationBuildConfig& config, CalibrationSet& out,
    const std::function<void(std::size_t, std::size_t)>& progress = {});

}  // namespace mt::similarity
