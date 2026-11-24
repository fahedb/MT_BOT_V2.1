// ---------------------------------------------------------------------------
// ChannelSet — the materialised channels for one symbol, keyed by channel id.
//
// Bridges the registry (what channels exist) and the data (their values). The
// legacy equivalent was IndicatorStorage plus the 180 flat arrays that
// TimeLineArrayBuilder produced, with the channel identity implicit in a
// four-level nested vector index.
// ---------------------------------------------------------------------------
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "mt/base/error.h"
#include "mt/data/bar.h"
#include "mt/data/series.h"
#include "mt/features/channel.h"
#include "mt/features/standardise.h"

namespace mt::features {

class ChannelSet {
public:
    ChannelSet() = default;

    [[nodiscard]] Status add(const std::string& channelId, data::Series series);
    [[nodiscard]] Result<const data::Series*> get(const std::string& channelId) const;
    [[nodiscard]] bool contains(const std::string& channelId) const;

    [[nodiscard]] std::size_t size() const noexcept { return m_byId.size(); }

    // Length of the underlying series. All channels must agree, which add()
    // enforces.
    [[nodiscard]] std::size_t length() const noexcept { return m_length; }

    // Every channel the registry names must be present, or scoring would
    // silently drop one.
    [[nodiscard]] Status validateAgainst(const ChannelRegistry& registry) const;

    // Standardised windows for every registered channel, ending at
    // `endIndex`, in registry order - exactly what Scorer::score and
    // EmbeddingBuilder::build expect.
    //
    // A channel whose window cannot be formed (too near the start of history)
    // yields an unusable window rather than an error, because that is a normal
    // condition during warm-up.
    [[nodiscard]] Result<std::vector<StandardisedWindow>> windowsEndingAt(
        const ChannelRegistry& registry, std::size_t endIndex, std::size_t windowLength) const;

private:
    std::unordered_map<std::string, data::Series> m_byId;
    std::size_t m_length = 0;
};

// The channel set the engine uses by default: everything computable from a
// single OHLC series, so it runs on whatever history is available without
// needing the multi-timeframe feed.
//
// Pairs with defaultEngineRegistry() below - the ids must match, and
// ChannelSet::validateAgainst checks that they do.
[[nodiscard]] ChannelRegistry defaultEngineRegistry(std::size_t paaSegments = 12);

[[nodiscard]] Result<ChannelSet> buildDefaultChannels(const data::BarSeries& bars);

}  // namespace mt::features
