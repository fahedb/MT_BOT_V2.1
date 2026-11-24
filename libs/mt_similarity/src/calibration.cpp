#include "mt/similarity/calibration.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>

namespace mt::similarity {
namespace {

constexpr std::array<char, 8> kMagic = {'M', 'T', 'C', 'A', 'L', '\0', '\0', '\0'};

template <typename T>
void writePod(std::ostream& out, const T& value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
bool readPod(std::istream& in, T& value) {
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
    return in.good();
}

}  // namespace

Result<ChannelCalibration> ChannelCalibration::fromSamples(std::vector<float> distances,
                                                           std::size_t bins) {
    if (bins == 0) {
        return invalidArgument("ChannelCalibration: bins must be > 0");
    }
    if (distances.size() < kMinSamples) {
        return invalidArgument("ChannelCalibration: need at least " +
                               std::to_string(kMinSamples) + " samples, got " +
                               std::to_string(distances.size()));
    }

    // Non-finite distances mean a broken channel upstream; dropping them
    // silently would hide that, so they are an error.
    for (const float d : distances) {
        if (!std::isfinite(d)) {
            return dataCorrupt("ChannelCalibration: non-finite distance sample");
        }
    }

    std::sort(distances.begin(), distances.end());

    ChannelCalibration calibration;
    std::vector<float> quantiles(bins + 1);
    const double lastIndex = static_cast<double>(distances.size() - 1);
    for (std::size_t k = 0; k <= bins; ++k) {
        const double p = static_cast<double>(k) / static_cast<double>(bins);
        const double pos = p * lastIndex;
        const std::size_t lo = static_cast<std::size_t>(std::floor(pos));
        const std::size_t hi = std::min(lo + 1, distances.size() - 1);
        const double frac = pos - static_cast<double>(lo);
        quantiles[k] = static_cast<float>(distances[lo] * (1.0 - frac) + distances[hi] * frac);
    }
    calibration.setQuantiles(std::move(quantiles));
    return calibration;
}

double ChannelCalibration::probabilityAtMost(double distanceValue) const {
    if (m_quantiles.empty()) {
        // No calibration: refuse to invent a probability. Callers check
        // CalibrationSet::validateAgainst before scoring, so reaching here
        // means a programming error, and 0 similarity is the safe answer.
        return 1.0;
    }
    const std::size_t binCount = m_quantiles.size() - 1;

    if (distanceValue <= static_cast<double>(m_quantiles.front())) {
        return 0.0;
    }
    if (distanceValue >= static_cast<double>(m_quantiles.back())) {
        return 1.0;
    }

    // Binary search for the bracketing boundaries, then interpolate.
    const auto it = std::upper_bound(m_quantiles.begin(), m_quantiles.end(),
                                     static_cast<float>(distanceValue));
    const std::size_t upper = static_cast<std::size_t>(std::distance(m_quantiles.begin(), it));
    const std::size_t lower = upper - 1;

    const double loD = static_cast<double>(m_quantiles[lower]);
    const double hiD = static_cast<double>(m_quantiles[upper]);
    const double span = hiD - loD;
    const double within = (span > 0.0) ? ((distanceValue - loD) / span) : 0.0;

    return (static_cast<double>(lower) + within) / static_cast<double>(binCount);
}

double ChannelCalibration::quantile(double probability) const {
    if (m_quantiles.empty()) {
        return 0.0;
    }
    const double p = std::clamp(probability, 0.0, 1.0);
    const std::size_t binCount = m_quantiles.size() - 1;
    const double pos = p * static_cast<double>(binCount);
    const std::size_t lo = static_cast<std::size_t>(std::floor(pos));
    const std::size_t hi = std::min(lo + 1, binCount);
    const double frac = pos - static_cast<double>(lo);
    return static_cast<double>(m_quantiles[lo]) * (1.0 - frac) +
           static_cast<double>(m_quantiles[hi]) * frac;
}

Status CalibrationSet::set(const std::string& channelId, ChannelCalibration calibration) {
    if (channelId.empty()) {
        return Status(invalidArgument("CalibrationSet::set: empty channel id"));
    }
    if (calibration.empty()) {
        return Status(invalidArgument("CalibrationSet::set: empty calibration for " + channelId));
    }
    m_byChannel[channelId] = std::move(calibration);
    return Status::success();
}

Result<const ChannelCalibration*> CalibrationSet::get(const std::string& channelId) const {
    const auto it = m_byChannel.find(channelId);
    if (it == m_byChannel.end()) {
        return notFound("CalibrationSet: no calibration for channel " + channelId);
    }
    return &it->second;
}

bool CalibrationSet::contains(const std::string& channelId) const {
    return m_byChannel.find(channelId) != m_byChannel.end();
}

Status CalibrationSet::validateAgainst(const features::ChannelRegistry& registry) const {
    if (m_fingerprint != registry.fingerprint()) {
        return Status(failedPrecondition(
            "CalibrationSet: fingerprint mismatch. This calibration was built for a different "
            "channel set or standardisation and would produce meaningless similarities. "
            "Rebuild it with apps/mtcalibrate."));
    }

    std::vector<std::string> missing;
    for (const auto& spec : registry.specs()) {
        if (spec.weight > 0.0 && !contains(spec.id)) {
            missing.push_back(spec.id);
        }
    }
    if (!missing.empty()) {
        return Status(notFound("CalibrationSet: " + std::to_string(missing.size()) +
                               " weighted channel(s) have no calibration, first: " +
                               missing.front()));
    }
    return Status::success();
}

Status CalibrationSet::save(const std::string& path) const {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return Status(ioFailure("CalibrationSet::save: cannot open " + path));
    }

    out.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
    writePod(out, kFormatVersion);
    writePod(out, m_fingerprint);
    writePod(out, static_cast<std::uint32_t>(m_byChannel.size()));

    // Sorted by id so the file is byte-reproducible for a given input, which
    // matters because the run manifest records its hash.
    std::vector<const std::pair<const std::string, ChannelCalibration>*> entries;
    entries.reserve(m_byChannel.size());
    for (const auto& entry : m_byChannel) {
        entries.push_back(&entry);
    }
    std::sort(entries.begin(), entries.end(),
              [](const auto* a, const auto* b) { return a->first < b->first; });

    for (const auto* entry : entries) {
        const std::string& id = entry->first;
        const std::vector<float>& quantiles = entry->second.quantiles();
        writePod(out, static_cast<std::uint32_t>(id.size()));
        out.write(id.data(), static_cast<std::streamsize>(id.size()));
        writePod(out, static_cast<std::uint32_t>(quantiles.size() - 1));
        out.write(reinterpret_cast<const char*>(quantiles.data()),
                  static_cast<std::streamsize>(quantiles.size() * sizeof(float)));
    }

    if (!out.good()) {
        return Status(ioFailure("CalibrationSet::save: write failed for " + path));
    }
    return Status::success();
}

Result<CalibrationSet> CalibrationSet::load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return ioFailure("CalibrationSet::load: cannot open " + path);
    }

    std::array<char, 8> magic{};
    in.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (!in.good() || magic != kMagic) {
        return dataCorrupt("CalibrationSet::load: bad magic in " + path);
    }

    std::uint32_t version = 0;
    if (!readPod(in, version)) {
        return dataCorrupt("CalibrationSet::load: truncated header");
    }
    if (version != kFormatVersion) {
        return dataCorrupt("CalibrationSet::load: version " + std::to_string(version) +
                           ", expected " + std::to_string(kFormatVersion));
    }

    std::uint64_t fingerprint = 0;
    std::uint32_t count = 0;
    if (!readPod(in, fingerprint) || !readPod(in, count)) {
        return dataCorrupt("CalibrationSet::load: truncated header");
    }

    CalibrationSet set(fingerprint);
    for (std::uint32_t c = 0; c < count; ++c) {
        std::uint32_t idLength = 0;
        if (!readPod(in, idLength) || idLength == 0 || idLength > 4096) {
            return dataCorrupt("CalibrationSet::load: bad channel id length");
        }
        std::string id(idLength, '\0');
        in.read(id.data(), static_cast<std::streamsize>(idLength));
        if (!in.good()) {
            return dataCorrupt("CalibrationSet::load: truncated channel id");
        }

        std::uint32_t bins = 0;
        if (!readPod(in, bins) || bins == 0) {
            return dataCorrupt("CalibrationSet::load: bad bin count for " + id);
        }
        std::vector<float> quantiles(static_cast<std::size_t>(bins) + 1);
        in.read(reinterpret_cast<char*>(quantiles.data()),
                static_cast<std::streamsize>(quantiles.size() * sizeof(float)));
        if (!in.good()) {
            return dataCorrupt("CalibrationSet::load: truncated quantiles for " + id);
        }

        ChannelCalibration calibration;
        calibration.setQuantiles(std::move(quantiles));
        if (Status status = set.set(id, std::move(calibration)); !status.ok()) {
            return status.error();
        }
    }
    return set;
}

Result<std::vector<std::pair<std::size_t, std::size_t>>> sampleWindowPairs(
    std::size_t windowCount, std::size_t windowLength,
    const CalibrationSamplingConfig& config) {
    if (windowLength == 0) {
        return invalidArgument("sampleWindowPairs: windowLength must be > 0");
    }
    const std::size_t minGap = windowLength + config.minSeparation;
    if (windowCount < 2 * minGap) {
        return invalidArgument("sampleWindowPairs: history of " + std::to_string(windowCount) +
                               " windows is too short to draw non-overlapping pairs separated "
                               "by " + std::to_string(minGap));
    }
    if (config.pairCount == 0) {
        return invalidArgument("sampleWindowPairs: pairCount must be > 0");
    }

    // splitmix64: deterministic, well distributed, no dependence on the
    // standard library's implementation-defined engines.
    std::uint64_t state = config.seed;
    const auto next = [&state]() {
        state += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    };

    std::vector<std::pair<std::size_t, std::size_t>> pairs;
    pairs.reserve(config.pairCount);

    // Rejection sampling. Bounded attempt count so a bad configuration fails
    // instead of spinning.
    const std::size_t maxAttempts = config.pairCount * 100 + 1000;
    std::size_t attempts = 0;
    while (pairs.size() < config.pairCount && attempts < maxAttempts) {
        ++attempts;
        const std::size_t i = static_cast<std::size_t>(next() % windowCount);
        const std::size_t j = static_cast<std::size_t>(next() % windowCount);
        const std::size_t gap = (i > j) ? (i - j) : (j - i);
        if (gap < minGap) {
            continue;  // overlapping or near-overlapping: trivially similar
        }
        pairs.emplace_back(i, j);
    }

    if (pairs.size() < config.pairCount) {
        return failedPrecondition("sampleWindowPairs: only drew " + std::to_string(pairs.size()) +
                                  " of " + std::to_string(config.pairCount) +
                                  " pairs; history is too short for this separation");
    }
    return pairs;
}

}  // namespace mt::similarity
