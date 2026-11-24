// ---------------------------------------------------------------------------
// Match scoring — docs/REFACTOR_PLAN.md section 6.5.
//
//     S = 100 * [ (1 - lambda) * SUM_c (w_c * s_c) + lambda * MIN_c (s_c) ]
//
// with SUM_c w_c = 1. The weighted mean is the main term; the min term is a
// consistency gate, so a candidate that matches beautifully on nine channels
// and contradicts on the tenth cannot win on volume alone.
//
// Three defects are fixed structurally rather than by care:
//
//  * B1 - the legacy cascade zeroed the score array at every stage
//    (BaseScoringEngine.cpp:108), so the score finally attached to a match
//    came only from the last stage and all earlier evidence was discarded.
//    Here a MatchScore is an immutable value, composed once and never
//    accumulated into a shared mutable buffer. There is no array to zero.
//  * S2 - the result is bounded in [0, 100] and independent of how many
//    channels were scanned, so a threshold keeps its meaning when the channel
//    set changes.
//  * B8 - weights come from the registry, which takes them from a learned
//    artefact. The legacy per-indicator weights were read from config and
//    then never used; the kernels applied a hardcoded constant.
//
// Every score keeps its per-channel breakdown, because a score that cannot be
// explained cannot be debugged.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "mt/base/error.h"
#include "mt/features/channel.h"
#include "mt/features/standardise.h"
#include "mt/similarity/calibration.h"

namespace mt::similarity {

struct ChannelScore {
    std::size_t channelIndex = 0;
    double rawDistance = 0.0;
    double similarity = 0.0;   // calibrated, [0, 1]
    double effectiveWeight = 0.0;  // after redistribution over usable channels
    bool usable = false;       // false when either window was unusable
    bool gatePassed = true;    // false when below this channel's hard gate
};

// Immutable. Constructed only by Scorer, which is what makes B1 impossible by
// design rather than by discipline.
class MatchScore {
public:
    MatchScore() = default;

    [[nodiscard]] double total() const noexcept { return m_total; }
    [[nodiscard]] double weightedMean() const noexcept { return m_weightedMean; }
    [[nodiscard]] double minSimilarity() const noexcept { return m_minSimilarity; }
    [[nodiscard]] bool gatesPassed() const noexcept { return m_gatesPassed; }

    // Share of the registry's weight that was actually usable. A low value
    // means most channels had incomplete windows, so the score rests on little
    // evidence even if it is high.
    [[nodiscard]] double usableWeight() const noexcept { return m_usableWeight; }

    [[nodiscard]] bool valid() const noexcept { return m_valid; }

    [[nodiscard]] const std::vector<ChannelScore>& channels() const noexcept {
        return m_channels;
    }

    // Weakest channels first: what to look at when a score is surprising.
    [[nodiscard]] std::vector<ChannelScore> weakestChannels(std::size_t count) const;

private:
    friend class Scorer;

    double m_total = 0.0;
    double m_weightedMean = 0.0;
    double m_minSimilarity = 0.0;
    double m_usableWeight = 0.0;
    bool m_gatesPassed = false;
    bool m_valid = false;
    std::vector<ChannelScore> m_channels;
};

struct ScoreConfig {
    // Weight on the min-consistency term, in [0, 0.5].
    double lambdaMin = 0.25;

    // A match whose usable channels carry less than this share of the total
    // weight is not scored at all. Scoring on a third of the evidence and
    // reporting it like a full match is how thin data becomes false
    // confidence.
    double minUsableWeightRatio = 0.5;

    // Band for DtwBand channels.
    std::size_t dtwBand = 3;
};

class Scorer {
public:
    // Validates the registry and calibration together, normalises the weights,
    // and keeps its own copy of the registry so a later mutation of the
    // caller's cannot change what a score means mid-run.
    [[nodiscard]] static Result<Scorer> create(features::ChannelRegistry registry,
                                               CalibrationSet calibration, ScoreConfig config);

    // `historyWindows` and `currentWindows` are indexed by channel, in
    // registry order. Both must have one entry per registered channel.
    [[nodiscard]] Result<MatchScore> score(
        std::span<const features::StandardisedWindow> historyWindows,
        std::span<const features::StandardisedWindow> currentWindows) const;

    [[nodiscard]] const features::ChannelRegistry& registry() const noexcept {
        return m_registry;
    }
    [[nodiscard]] const ScoreConfig& config() const noexcept { return m_config; }

    // The highest total a perfect match can reach, i.e. 100. Stated as a
    // function so call sites read as intent rather than as a magic number.
    [[nodiscard]] static constexpr double maxScore() noexcept { return 100.0; }

private:
    Scorer(features::ChannelRegistry registry, CalibrationSet calibration, ScoreConfig config)
        : m_registry(std::move(registry)),
          m_calibration(std::move(calibration)),
          m_config(config) {}

    features::ChannelRegistry m_registry;
    CalibrationSet m_calibration;
    ScoreConfig m_config;
};

}  // namespace mt::similarity
