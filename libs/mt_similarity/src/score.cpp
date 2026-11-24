#include "mt/similarity/score.h"

#include <algorithm>
#include <cmath>

#include "mt/similarity/distance.h"

namespace mt::similarity {

std::vector<ChannelScore> MatchScore::weakestChannels(std::size_t count) const {
    std::vector<ChannelScore> usable;
    usable.reserve(m_channels.size());
    for (const auto& channel : m_channels) {
        if (channel.usable && channel.effectiveWeight > 0.0) {
            usable.push_back(channel);
        }
    }
    std::sort(usable.begin(), usable.end(), [](const ChannelScore& a, const ChannelScore& b) {
        return a.similarity < b.similarity;
    });
    if (usable.size() > count) {
        usable.resize(count);
    }
    return usable;
}

Result<Scorer> Scorer::create(features::ChannelRegistry registry, CalibrationSet calibration,
                              ScoreConfig config) {
    if (registry.empty()) {
        return invalidArgument("Scorer::create: empty channel registry");
    }
    if (config.lambdaMin < 0.0 || config.lambdaMin > 0.5) {
        return invalidArgument("Scorer::create: lambdaMin must be in [0, 0.5], got " +
                               std::to_string(config.lambdaMin));
    }
    if (config.minUsableWeightRatio < 0.0 || config.minUsableWeightRatio > 1.0) {
        return invalidArgument("Scorer::create: minUsableWeightRatio must be in [0, 1]");
    }

    Result<std::size_t> active = registry.normaliseWeights();
    if (!active) {
        return active.error();
    }
    if (active.value() == 0) {
        return invalidArgument("Scorer::create: every channel has zero weight");
    }

    // Checked once here, not per match: a partial calibration would silently
    // drop channels from every total.
    if (Status status = calibration.validateAgainst(registry); !status.ok()) {
        return status.error();
    }

    return Scorer(std::move(registry), std::move(calibration), config);
}

Result<MatchScore> Scorer::score(std::span<const features::StandardisedWindow> historyWindows,
                                 std::span<const features::StandardisedWindow> currentWindows)
    const {
    const std::size_t channelCount = m_registry.size();
    if (historyWindows.size() != channelCount || currentWindows.size() != channelCount) {
        return invalidArgument("Scorer::score: expected " + std::to_string(channelCount) +
                               " windows per side, got " + std::to_string(historyWindows.size()) +
                               " and " + std::to_string(currentWindows.size()));
    }

    MatchScore result;
    result.m_channels.resize(channelCount);

    // Pass 1: raw distances and calibrated similarities for the channels whose
    // windows are usable on both sides.
    double usableWeight = 0.0;
    for (std::size_t c = 0; c < channelCount; ++c) {
        const features::ChannelSpec& spec = m_registry[c];
        ChannelScore& channel = result.m_channels[c];
        channel.channelIndex = c;

        if (spec.weight <= 0.0) {
            continue;  // pruned by the learned weights; not an error
        }
        if (!historyWindows[c].usable || !currentWindows[c].usable) {
            channel.usable = false;
            continue;
        }

        Result<double> raw = distance(historyWindows[c].values, currentWindows[c].values,
                                      spec.distance, m_config.dtwBand);
        if (!raw) {
            return raw.error();
        }

        const Result<const ChannelCalibration*> calibration = m_calibration.get(spec.id);
        if (!calibration) {
            return calibration.error();
        }

        channel.rawDistance = raw.value();
        channel.similarity = std::clamp(calibration.value()->similarity(raw.value()), 0.0, 1.0);
        channel.usable = true;
        usableWeight += spec.weight;
    }

    result.m_usableWeight = usableWeight;

    if (usableWeight < m_config.minUsableWeightRatio) {
        // Not an error: a normal consequence of a data gap. The match simply
        // cannot be scored, and the caller must drop it rather than treat a
        // zero as a low score.
        result.m_valid = false;
        return result;
    }

    // Pass 2: redistribute the weight of unusable channels over the usable
    // ones, so the weighted mean stays on the same [0, 1] scale regardless of
    // how many channels dropped out.
    double weightedSum = 0.0;
    double minSimilarity = 1.0;
    bool gatesPassed = true;
    bool anyUsable = false;

    for (std::size_t c = 0; c < channelCount; ++c) {
        const features::ChannelSpec& spec = m_registry[c];
        ChannelScore& channel = result.m_channels[c];
        if (!channel.usable || spec.weight <= 0.0) {
            continue;
        }
        channel.effectiveWeight = spec.weight / usableWeight;
        weightedSum += channel.effectiveWeight * channel.similarity;

        if (channel.similarity < minSimilarity) {
            minSimilarity = channel.similarity;
        }
        anyUsable = true;

        if (spec.gate > 0.0) {
            channel.gatePassed = (channel.similarity >= spec.gate);
            if (!channel.gatePassed) {
                gatesPassed = false;
            }
        }
    }

    if (!anyUsable) {
        result.m_valid = false;
        return result;
    }

    result.m_weightedMean = weightedSum;
    result.m_minSimilarity = minSimilarity;
    result.m_gatesPassed = gatesPassed;
    result.m_total = maxScore() * ((1.0 - m_config.lambdaMin) * weightedSum +
                                   m_config.lambdaMin * minSimilarity);
    result.m_valid = true;
    return result;
}

}  // namespace mt::similarity
