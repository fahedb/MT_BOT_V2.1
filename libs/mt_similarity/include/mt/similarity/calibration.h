// ---------------------------------------------------------------------------
// Calibration — docs/REFACTOR_PLAN.md section 6.4, the central new idea.
//
// A raw distance is in that channel's own units and says nothing about how
// *improbable* the match is. An L1 of 0.4 might be remarkable on one channel
// and unremarkable on another. The legacy engine summed such numbers across
// 180 channels and compared the total to a hand-set threshold (findings S2,
// S3), which is why no threshold could ever be set rationally.
//
// Here each channel's distance is mapped through its own empirical
// distribution, measured offline over random non-overlapping window pairs:
//
//     p_c = F_c(d_c)          probability a random pair is at least this close
//     s_c = 1 - p_c           calibrated similarity, in [0, 1]
//
// This buys three things:
//   * comparability   - s_c means the same on every channel;
//   * meaningful thresholds - "keep matches above 90" is a statement about
//     rarity, so the engine can legitimately return *nothing* when the
//     present has no historical analogue (finding S7);
//   * interpretability - a match can be reported as "top 0.3% of channel-level
//     similarity" rather than as "score 47000".
//
// A calibration is only valid for the channel set and standardisation it was
// built from, so it records the registry fingerprint and refuses to load
// against a different one.
// ---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "mt/base/error.h"
#include "mt/features/channel.h"

namespace mt::similarity {

// The empirical CDF of one channel's distances, stored as quantile
// boundaries. 1024 bins is small (4 KB per channel) and accurate to ~0.1% in
// probability, which is finer than anything downstream needs.
class ChannelCalibration {
public:
    static constexpr std::size_t kDefaultBins = 1024;

    ChannelCalibration() = default;

    // `distances` is consumed and sorted. At least kMinSamples are required:
    // a CDF from a handful of pairs would be noise presented as precision.
    static constexpr std::size_t kMinSamples = 1000;
    [[nodiscard]] static Result<ChannelCalibration> fromSamples(
        std::vector<float> distances, std::size_t bins = kDefaultBins);

    [[nodiscard]] bool empty() const noexcept { return m_quantiles.empty(); }
    [[nodiscard]] std::size_t bins() const noexcept {
        return m_quantiles.empty() ? 0 : m_quantiles.size() - 1;
    }

    // F(d): the fraction of random pairs at distance <= d, linearly
    // interpolated between quantile boundaries. Clamped to [0, 1].
    [[nodiscard]] double probabilityAtMost(double distanceValue) const;

    // 1 - F(d). 1.0 means closer than essentially every random pair.
    [[nodiscard]] double similarity(double distanceValue) const {
        return 1.0 - probabilityAtMost(distanceValue);
    }

    // Diagnostics for reports: the distance at a given probability.
    [[nodiscard]] double quantile(double probability) const;

    [[nodiscard]] const std::vector<float>& quantiles() const noexcept { return m_quantiles; }
    void setQuantiles(std::vector<float> quantiles) { m_quantiles = std::move(quantiles); }

private:
    // size == bins + 1, non-decreasing. m_quantiles[k] is the distance at
    // probability k / bins.
    std::vector<float> m_quantiles;
};

// Calibrations for a whole registry, plus the fingerprint they belong to.
class CalibrationSet {
public:
    CalibrationSet() = default;
    explicit CalibrationSet(std::uint64_t registryFingerprint)
        : m_fingerprint(registryFingerprint) {}

    [[nodiscard]] std::uint64_t fingerprint() const noexcept { return m_fingerprint; }
    void setFingerprint(std::uint64_t value) { m_fingerprint = value; }

    [[nodiscard]] std::size_t size() const noexcept { return m_byChannel.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_byChannel.empty(); }

    [[nodiscard]] Status set(const std::string& channelId, ChannelCalibration calibration);
    [[nodiscard]] Result<const ChannelCalibration*> get(const std::string& channelId) const;
    [[nodiscard]] bool contains(const std::string& channelId) const;

    // Every channel with a non-zero weight must have a calibration. Scoring
    // against a partial set would silently drop channels from the total, so
    // this is checked up front rather than per match.
    [[nodiscard]] Status validateAgainst(const features::ChannelRegistry& registry) const;

    // Binary format, little-endian:
    //   magic "MTCAL\0\0\0" (8 bytes)
    //   uint32 version
    //   uint64 registry fingerprint
    //   uint32 channel count
    //   per channel: uint32 id length, id bytes, uint32 bin count,
    //                (bins + 1) float32 quantiles
    [[nodiscard]] Status save(const std::string& path) const;
    [[nodiscard]] static Result<CalibrationSet> load(const std::string& path);

    static constexpr std::uint32_t kFormatVersion = 1;

private:
    std::uint64_t m_fingerprint = 0;
    std::unordered_map<std::string, ChannelCalibration> m_byChannel;
};

// Builds a calibration by sampling random non-overlapping window pairs.
//
// `sampleDistance(i, j)` must return the raw distance between history windows
// beginning at i and j for one channel, or an error. `windowCount` is how many
// windows exist, `windowLength` how long each is: pairs are rejected when they
// overlap, since an overlapping pair is trivially similar and would bias the
// null distribution towards small distances (which would make every real match
// look unremarkable).
struct CalibrationSamplingConfig {
    std::size_t pairCount = 200000;
    std::uint64_t seed = 0x5EEDCA11;
    std::size_t bins = ChannelCalibration::kDefaultBins;

    // Extra separation required between the two windows of a pair, on top of
    // non-overlap. Adjacent windows share most of their data even when they do
    // not strictly overlap.
    std::size_t minSeparation = 0;
};

[[nodiscard]] Result<std::vector<std::pair<std::size_t, std::size_t>>> sampleWindowPairs(
    std::size_t windowCount, std::size_t windowLength,
    const CalibrationSamplingConfig& config);

}  // namespace mt::similarity
