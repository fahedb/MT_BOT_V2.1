#include "mt/similarity/calibrate.h"

#include <utility>

#include "mt/similarity/distance.h"

namespace mt::similarity {

Result<CalibrationBuildReport> buildCalibration(
    const features::ChannelRegistry& registry, const features::ChannelSet& channels,
    const CalibrationBuildConfig& config, CalibrationSet& out,
    const std::function<void(std::size_t, std::size_t)>& progress) {
    if (registry.empty()) {
        return invalidArgument("buildCalibration: empty registry");
    }
    if (config.windowLength < 2) {
        return invalidArgument("buildCalibration: windowLength must be >= 2");
    }
    if (Status status = channels.validateAgainst(registry); !status.ok()) {
        return status.error();
    }

    const std::size_t seriesLength = channels.length();
    if (seriesLength <= config.windowLength) {
        return invalidArgument("buildCalibration: history of " + std::to_string(seriesLength) +
                               " samples is shorter than the window length");
    }
    const std::size_t windowCount = seriesLength - config.windowLength + 1;

    CalibrationSamplingConfig sampling = config.sampling;
    if (sampling.minSeparation == 0) {
        // Adjacent windows share nearly all their data even when they do not
        // strictly overlap, and including them would bias the null
        // distribution towards small distances - making every genuine match
        // look unremarkable.
        sampling.minSeparation = config.windowLength;
    }

    Result<std::vector<std::pair<std::size_t, std::size_t>>> pairs =
        sampleWindowPairs(windowCount, config.windowLength, sampling);
    if (!pairs) {
        return pairs.error();
    }

    CalibrationBuildReport report;
    report.pairsRequested = pairs.value().size();

    out = CalibrationSet(registry.fingerprint());

    const std::size_t channelCount = registry.size();
    for (std::size_t c = 0; c < channelCount; ++c) {
        const features::ChannelSpec& spec = registry[c];

        Result<const data::Series*> series = channels.get(spec.id);
        if (!series) {
            return series.error();
        }

        std::vector<float> distances;
        distances.reserve(pairs.value().size());
        std::size_t skipped = 0;

        for (const auto& [i, j] : pairs.value()) {
            Result<data::WindowView> viewA = series.value()->windowFrom(i, config.windowLength);
            Result<data::WindowView> viewB = series.value()->windowFrom(j, config.windowLength);
            if (!viewA || !viewB) {
                ++skipped;
                continue;
            }
            Result<features::StandardisedWindow> a =
                features::standardise(viewA.value(), spec.standardise);
            Result<features::StandardisedWindow> b =
                features::standardise(viewB.value(), spec.standardise);
            if (!a || !b) {
                ++skipped;
                continue;
            }
            if (!a.value().usable || !b.value().usable) {
                ++skipped;
                continue;
            }
            Result<double> d =
                distance(a.value().values, b.value().values, spec.distance);
            if (!d) {
                return d.error();
            }
            distances.push_back(static_cast<float>(d.value()));
        }

        const double skipRatio = static_cast<double>(skipped) /
                                 static_cast<double>(pairs.value().size());
        if (skipRatio > config.maxSkipRatio) {
            return failedPrecondition(
                "buildCalibration: channel " + spec.id + " skipped " +
                std::to_string(static_cast<int>(skipRatio * 100.0)) +
                "% of sampled pairs (warm-up or data gaps). A CDF from the remainder would "
                "not describe this channel.");
        }

        Result<ChannelCalibration> calibration =
            ChannelCalibration::fromSamples(std::move(distances), sampling.bins);
        if (!calibration) {
            return calibration.error();
        }
        if (Status status = out.set(spec.id, std::move(calibration).value()); !status.ok()) {
            return status.error();
        }

        ++report.channelsCalibrated;
        if (progress) {
            progress(c + 1, channelCount);
        }
    }

    report.pairsUsable = report.pairsRequested;
    return report;
}

}  // namespace mt::similarity
