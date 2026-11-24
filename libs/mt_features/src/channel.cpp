#include "mt/features/channel.h"

#include <algorithm>
#include <array>
#include <numeric>

namespace mt::features {

std::string_view toString(DistanceKind kind) noexcept {
    switch (kind) {
        case DistanceKind::L1Shape:     return "l1_shape";
        case DistanceKind::L2Shape:     return "l2_shape";
        case DistanceKind::Correlation: return "correlation";
        case DistanceKind::Cosine:      return "cosine";
        case DistanceKind::DtwBand:     return "dtw_band";
    }
    return "l1_shape";
}

Result<DistanceKind> distanceKindFromString(std::string_view name) {
    if (name == "l1_shape")    return DistanceKind::L1Shape;
    if (name == "l2_shape")    return DistanceKind::L2Shape;
    if (name == "correlation") return DistanceKind::Correlation;
    if (name == "cosine")      return DistanceKind::Cosine;
    if (name == "dtw_band")    return DistanceKind::DtwBand;
    return invalidArgument("unknown distance kind: " + std::string(name));
}

Status ChannelRegistry::add(ChannelSpec spec) {
    if (spec.id.empty()) {
        return Status(invalidArgument("ChannelRegistry::add: empty channel id"));
    }
    if (spec.weight < 0.0) {
        return Status(invalidArgument("ChannelRegistry::add: negative weight for " + spec.id));
    }
    if (m_byId.find(spec.id) != m_byId.end()) {
        return Status(Error(ErrorCode::AlreadyExists,
                            "ChannelRegistry::add: duplicate channel id " + spec.id));
    }
    m_byId.emplace(spec.id, m_specs.size());
    m_specs.push_back(std::move(spec));
    return Status::success();
}

Result<std::size_t> ChannelRegistry::indexOf(const std::string& id) const {
    const auto it = m_byId.find(id);
    if (it == m_byId.end()) {
        return notFound("ChannelRegistry: no channel " + id);
    }
    return it->second;
}

Result<const ChannelSpec*> ChannelRegistry::find(const std::string& id) const {
    const auto it = m_byId.find(id);
    if (it == m_byId.end()) {
        return notFound("ChannelRegistry: no channel " + id);
    }
    return &m_specs[it->second];
}

Status ChannelRegistry::applyWeights(const std::unordered_map<std::string, double>& weights) {
    std::vector<std::string> unknown;
    for (const auto& [id, weight] : weights) {
        const auto it = m_byId.find(id);
        if (it == m_byId.end()) {
            unknown.push_back(id);
            continue;
        }
        if (weight < 0.0) {
            return Status(invalidArgument("ChannelRegistry::applyWeights: negative weight for " +
                                          id));
        }
        m_specs[it->second].weight = weight;
    }
    if (!unknown.empty()) {
        std::string message = "ChannelRegistry::applyWeights: " + std::to_string(unknown.size()) +
                              " weight(s) for unregistered channels, first: " + unknown.front();
        return Status(notFound(std::move(message)));
    }
    return Status::success();
}

Result<std::size_t> ChannelRegistry::normaliseWeights() {
    if (m_specs.empty()) {
        return invalidArgument("ChannelRegistry::normaliseWeights: empty registry");
    }

    // Cluster-first weighting (finding S4). A channel with an empty cluster is
    // its own cluster, keyed by its id so it cannot collide.
    std::unordered_map<std::string, std::vector<std::size_t>> byCluster;
    for (std::size_t i = 0; i < m_specs.size(); ++i) {
        const std::string key =
            m_specs[i].cluster.empty() ? ("#" + m_specs[i].id) : m_specs[i].cluster;
        byCluster[key].push_back(i);
    }

    // Each cluster's claim is the mean of its members' weights, so a cluster
    // does not gain influence merely by having more members.
    double totalClusterWeight = 0.0;
    std::unordered_map<std::string, double> clusterWeight;
    for (const auto& [key, members] : byCluster) {
        double sum = 0.0;
        for (const std::size_t i : members) {
            sum += m_specs[i].weight;
        }
        const double mean = sum / static_cast<double>(members.size());
        clusterWeight[key] = mean;
        totalClusterWeight += mean;
    }

    if (!(totalClusterWeight > 0.0)) {
        return invalidArgument("ChannelRegistry::normaliseWeights: all weights are zero");
    }

    std::size_t active = 0;
    for (const auto& [key, members] : byCluster) {
        const double share = clusterWeight[key] / totalClusterWeight;
        // Split the cluster's share among members in proportion to their own
        // weights, so a zeroed channel stays zeroed.
        double memberSum = 0.0;
        for (const std::size_t i : members) {
            memberSum += m_specs[i].weight;
        }
        for (const std::size_t i : members) {
            m_specs[i].weight = (memberSum > 0.0)
                                    ? share * (m_specs[i].weight / memberSum)
                                    : share / static_cast<double>(members.size());
            if (m_specs[i].weight > 0.0) {
                ++active;
            }
        }
    }
    return active;
}

std::vector<std::size_t> ChannelRegistry::embeddingChannels() const {
    std::vector<std::size_t> out;
    for (std::size_t i = 0; i < m_specs.size(); ++i) {
        if (m_specs[i].inEmbedding && m_specs[i].weight > 0.0) {
            out.push_back(i);
        }
    }
    return out;
}

std::vector<std::size_t> ChannelRegistry::gatedChannels() const {
    std::vector<std::size_t> out;
    for (std::size_t i = 0; i < m_specs.size(); ++i) {
        if (m_specs[i].gate > 0.0) {
            out.push_back(i);
        }
    }
    return out;
}

std::vector<std::string> ChannelRegistry::clusters() const {
    std::vector<std::string> out;
    for (const auto& spec : m_specs) {
        if (spec.cluster.empty()) {
            continue;
        }
        if (std::find(out.begin(), out.end(), spec.cluster) == out.end()) {
            out.push_back(spec.cluster);
        }
    }
    return out;
}

std::uint64_t ChannelRegistry::fingerprint() const {
    // FNV-1a over the fields that affect what a distance means. Weights are
    // excluded on purpose: re-weighting does not invalidate a calibration,
    // whereas changing standardisation or window shape does.
    std::uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](std::uint64_t value) {
        for (int byte = 0; byte < 8; ++byte) {
            hash ^= (value >> (byte * 8)) & 0xFFULL;
            hash *= 1099511628211ULL;
        }
    };
    const auto mixString = [&hash](std::string_view s) {
        for (const char c : s) {
            hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            hash *= 1099511628211ULL;
        }
    };

    for (const auto& spec : m_specs) {
        mixString(spec.id);
        mixString(toString(spec.distance));
        mix(static_cast<std::uint64_t>(spec.standardise.mode));
        mix(static_cast<std::uint64_t>(spec.standardise.paaSegments));
        mix(static_cast<std::uint64_t>(spec.standardise.clampSigma * 1000.0F));
        mix(static_cast<std::uint64_t>(spec.standardise.minValidRatio * 1000.0));
    }
    return hash;
}

// --- ready-made registries ------------------------------------------------

ChannelRegistry legacyGridRegistry(std::size_t paaSegments) {
    static constexpr std::array<const char*, 3> kIndicators = {"ema", "rsi", "zlsma"};
    static constexpr std::array<int, 5> kLengths = {10, 15, 20, 25, 30};
    static constexpr std::array<const char*, 6> kTimeframes = {"M1", "M5", "M15",
                                                               "M30", "H1", "H4"};
    static constexpr std::array<const char*, 2> kValueTypes = {"value", "slope"};

    ChannelRegistry registry;
    for (const char* indicator : kIndicators) {
        for (const int length : kLengths) {
            for (const char* timeframe : kTimeframes) {
                for (const char* valueType : kValueTypes) {
                    ChannelSpec spec;
                    spec.id = std::string(indicator) + "." + std::to_string(length) + "." +
                              timeframe + "." + valueType;
                    spec.standardise.mode = StandardiseMode::RobustZ;
                    spec.standardise.paaSegments = paaSegments;
                    spec.distance = DistanceKind::L1Shape;
                    // One cluster per (indicator, value type): the five
                    // lengths and six timeframes of one indicator are highly
                    // correlated and should share influence, not multiply it.
                    spec.cluster = std::string(indicator) + "." + valueType;
                    spec.inEmbedding = true;
                    spec.weight = 1.0;
                    // Registry construction cannot fail here: ids are unique
                    // by construction and weights are positive.
                    (void)registry.add(std::move(spec));
                }
            }
        }
    }
    return registry;
}

ChannelRegistry researchBaselineRegistry(std::size_t paaSegments) {
    ChannelRegistry registry;

    const auto addSpec = [&registry, paaSegments](const char* id, const char* cluster,
                                                  StandardiseMode mode, DistanceKind distance,
                                                  bool paa) {
        ChannelSpec spec;
        spec.id = id;
        spec.cluster = cluster;
        spec.standardise.mode = mode;
        spec.standardise.paaSegments = paa ? paaSegments : 0;
        spec.distance = distance;
        spec.weight = 1.0;
        (void)registry.add(std::move(spec));
    };

    // Shape channels: a window each, reduced by PAA.
    addSpec("price.close.shape",   "price",      StandardiseMode::RobustZ,    DistanceKind::L1Shape, true);
    addSpec("price.range.shape",   "volatility", StandardiseMode::RobustZ,    DistanceKind::L1Shape, true);
    addSpec("candle.body.shape",   "structure",  StandardiseMode::RobustZ,    DistanceKind::L1Shape, true);
    addSpec("ema.20.M1.shape",     "ema",        StandardiseMode::RobustZ,    DistanceKind::L1Shape, true);
    addSpec("ema.20.M15.shape",    "ema",        StandardiseMode::RobustZ,    DistanceKind::L1Shape, true);
    addSpec("rsi.14.M1.shape",     "rsi",        StandardiseMode::RobustZ,    DistanceKind::L1Shape, true);

    // Scalar-like channels: already dimensionless or bounded, so no PAA and a
    // gentler standardisation. These are the ports of the Python extractors
    // that returned a handful of numbers rather than a curve.
    addSpec("returns.multi",       "returns",    StandardiseMode::None,       DistanceKind::L1Shape, false);
    addSpec("volatility.ratio",    "volatility", StandardiseMode::None,       DistanceKind::L1Shape, false);
    addSpec("range.location",      "location",   StandardiseMode::None,       DistanceKind::L1Shape, false);
    addSpec("rsi.14.M1.level",     "rsi",        StandardiseMode::CentreOnly, DistanceKind::L1Shape, false);
    addSpec("ema.distance",        "ema",        StandardiseMode::None,       DistanceKind::L1Shape, false);
    addSpec("h1.trend.state",      "trend",      StandardiseMode::None,       DistanceKind::L1Shape, false);

    return registry;
}

}  // namespace mt::features
