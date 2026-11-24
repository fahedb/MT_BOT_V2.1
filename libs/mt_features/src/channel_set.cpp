#include "mt/features/channel_set.h"

#include <utility>

#include "mt/features/indicators.h"

namespace mt::features {

Status ChannelSet::add(const std::string& channelId, data::Series series) {
    if (channelId.empty()) {
        return Status(invalidArgument("ChannelSet::add: empty channel id"));
    }
    if (series.empty()) {
        return Status(invalidArgument("ChannelSet::add: empty series for " + channelId));
    }
    if (m_length == 0) {
        m_length = series.size();
    } else if (series.size() != m_length) {
        // Channels of different lengths cannot be windowed at a common index,
        // and silently truncating is how comparisons end up misaligned in
        // time.
        return Status(invalidArgument("ChannelSet::add: " + channelId + " has " +
                                      std::to_string(series.size()) +
                                      " samples, expected " + std::to_string(m_length)));
    }
    m_byId.insert_or_assign(channelId, std::move(series));
    return Status::success();
}

Result<const data::Series*> ChannelSet::get(const std::string& channelId) const {
    const auto it = m_byId.find(channelId);
    if (it == m_byId.end()) {
        return notFound("ChannelSet: no series for channel " + channelId);
    }
    return &it->second;
}

bool ChannelSet::contains(const std::string& channelId) const {
    return m_byId.find(channelId) != m_byId.end();
}

Status ChannelSet::validateAgainst(const ChannelRegistry& registry) const {
    std::vector<std::string> missing;
    for (const auto& spec : registry.specs()) {
        if (!contains(spec.id)) {
            missing.push_back(spec.id);
        }
    }
    if (!missing.empty()) {
        return Status(notFound("ChannelSet: " + std::to_string(missing.size()) +
                               " registered channel(s) have no data, first: " +
                               missing.front()));
    }
    return Status::success();
}

Result<std::vector<StandardisedWindow>> ChannelSet::windowsEndingAt(
    const ChannelRegistry& registry, std::size_t endIndex, std::size_t windowLength) const {
    std::vector<StandardisedWindow> out;
    out.reserve(registry.size());

    for (const auto& spec : registry.specs()) {
        const auto it = m_byId.find(spec.id);
        if (it == m_byId.end()) {
            return notFound("ChannelSet::windowsEndingAt: no series for channel " + spec.id);
        }

        Result<data::WindowView> view = it->second.window(endIndex, windowLength);
        if (!view) {
            // Not enough history yet: a normal warm-up condition, so the
            // window is unusable rather than an error.
            out.emplace_back();
            continue;
        }

        Result<StandardisedWindow> standardised = standardise(view.value(), spec.standardise);
        if (!standardised) {
            return standardised.error();
        }
        out.push_back(std::move(standardised).value());
    }
    return out;
}

ChannelRegistry defaultEngineRegistry(std::size_t paaSegments) {
    ChannelRegistry registry;

    const auto addSpec = [&registry, paaSegments](const char* id, const char* cluster,
                                                  StandardiseMode mode, DistanceKind distance,
                                                  double weight) {
        ChannelSpec spec;
        spec.id = id;
        spec.cluster = cluster;
        spec.standardise.mode = mode;
        spec.standardise.paaSegments = paaSegments;
        spec.distance = distance;
        spec.weight = weight;
        (void)registry.add(std::move(spec));
    };

    // Price shape: what the pattern looks like.
    addSpec("price.close.shape", "price", StandardiseMode::RobustZ, DistanceKind::L1Shape, 1.0);

    // Structure: how the bars themselves are formed.
    addSpec("price.range.shape", "volatility", StandardiseMode::RobustZ,
            DistanceKind::L1Shape, 1.0);
    addSpec("candle.body.shape", "structure", StandardiseMode::RobustZ, DistanceKind::L1Shape,
            1.0);
    addSpec("candle.location", "structure", StandardiseMode::None, DistanceKind::L1Shape, 1.0);

    // Trend: two lengths, one cluster, so they share influence rather than
    // doubling it (finding S4).
    addSpec("ema.20.shape", "ema", StandardiseMode::RobustZ, DistanceKind::L1Shape, 1.0);
    addSpec("ema.50.shape", "ema", StandardiseMode::RobustZ, DistanceKind::L1Shape, 1.0);

    // Trend direction, compared by correlation so only the shape of the slope
    // matters and its amplitude does not.
    addSpec("ema.20.slope.shape", "ema", StandardiseMode::RobustZ, DistanceKind::Correlation,
            1.0);

    // Momentum and volatility level.
    addSpec("rsi.14.shape", "rsi", StandardiseMode::RobustZ, DistanceKind::L1Shape, 1.0);
    addSpec("atr.14.shape", "volatility", StandardiseMode::RobustZ, DistanceKind::L1Shape, 1.0);

    return registry;
}

Result<ChannelSet> buildDefaultChannels(const data::BarSeries& bars) {
    if (bars.empty()) {
        return invalidArgument("buildDefaultChannels: empty bar series");
    }

    ChannelSet set;

    const auto addOrFail = [&set](const char* id, data::Series series) -> Status {
        return set.add(id, std::move(series));
    };

    if (Status status = addOrFail("price.close.shape", closeSeries(bars)); !status.ok()) {
        return status.error();
    }
    if (Status status = addOrFail("price.range.shape", rangeSeries(bars)); !status.ok()) {
        return status.error();
    }
    if (Status status = addOrFail("candle.body.shape", bodySeries(bars)); !status.ok()) {
        return status.error();
    }
    if (Status status = addOrFail("candle.location", closeLocationSeries(bars)); !status.ok()) {
        return status.error();
    }

    Result<data::Series> ema20 = ema(bars, 20);
    if (!ema20) {
        return ema20.error();
    }
    Result<data::Series> ema50 = ema(bars, 50);
    if (!ema50) {
        return ema50.error();
    }
    const data::Series ema20Slope = difference(ema20.value());

    if (Status status = addOrFail("ema.20.shape", ema20.value()); !status.ok()) {
        return status.error();
    }
    if (Status status = addOrFail("ema.50.shape", std::move(ema50).value()); !status.ok()) {
        return status.error();
    }
    if (Status status = addOrFail("ema.20.slope.shape", ema20Slope); !status.ok()) {
        return status.error();
    }

    Result<data::Series> rsi14 = rsi(bars, 14);
    if (!rsi14) {
        return rsi14.error();
    }
    if (Status status = addOrFail("rsi.14.shape", std::move(rsi14).value()); !status.ok()) {
        return status.error();
    }

    Result<data::Series> atr14 = atr(bars, 14);
    if (!atr14) {
        return atr14.error();
    }
    if (Status status = addOrFail("atr.14.shape", std::move(atr14).value()); !status.ok()) {
        return status.error();
    }

    return set;
}

}  // namespace mt::features
