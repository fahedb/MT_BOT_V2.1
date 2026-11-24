#include "mt/retrieval/embedding.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>

namespace mt::retrieval {
namespace {

constexpr std::array<char, 8> kMagic = {'M', 'T', 'E', 'M', 'B', '\0', '\0', '\0'};

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

Result<EmbeddingBuilder> EmbeddingBuilder::create(const features::ChannelRegistry& registry,
                                                  EmbeddingConfig config) {
    EmbeddingBuilder builder;
    builder.m_channels = registry.embeddingChannels();

    if (builder.m_channels.empty()) {
        return invalidArgument(
            "EmbeddingBuilder: no channel participates in the embedding. Every candidate "
            "channel is either excluded or has zero weight.");
    }

    std::size_t offset = 0;
    for (const std::size_t channelIndex : builder.m_channels) {
        const features::ChannelSpec& spec = registry[channelIndex];
        if (spec.standardise.paaSegments == 0) {
            // Without PAA the block length is the window length, which varies
            // by search scale; an embedding must be fixed-size.
            return invalidArgument("EmbeddingBuilder: channel " + spec.id +
                                   " has no PAA segment count, so its embedding block has no "
                                   "fixed length. Set paaSegments or exclude it from the "
                                   "embedding.");
        }
        builder.m_offsets.push_back(offset);
        offset += spec.standardise.paaSegments;

        const double weight = config.scaleByWeight ? std::max(spec.weight, 0.0) : 1.0;
        builder.m_scales.push_back(static_cast<float>(std::sqrt(weight)));
    }

    builder.m_dimension = offset;
    return builder;
}

Result<std::vector<float>> EmbeddingBuilder::build(
    std::span<const features::StandardisedWindow> windows) const {
    std::vector<float> out(m_dimension, 0.0F);

    for (std::size_t c = 0; c < m_channels.size(); ++c) {
        const std::size_t channelIndex = m_channels[c];
        if (channelIndex >= windows.size()) {
            return invalidArgument("EmbeddingBuilder::build: window list is shorter than the "
                                   "registry");
        }
        const features::StandardisedWindow& window = windows[channelIndex];
        if (!window.usable) {
            // A hole in the vector would make this window spuriously close to
            // everything, so an incomplete embedding is refused outright.
            return failedPrecondition("EmbeddingBuilder::build: channel index " +
                                      std::to_string(channelIndex) +
                                      " is unusable, so no embedding exists for this window");
        }

        const std::size_t begin = m_offsets[c];
        const std::size_t length = (c + 1 < m_offsets.size())
                                       ? (m_offsets[c + 1] - begin)
                                       : (m_dimension - begin);
        if (window.values.size() != length) {
            return dataCorrupt("EmbeddingBuilder::build: channel index " +
                               std::to_string(channelIndex) + " produced " +
                               std::to_string(window.values.size()) + " values, expected " +
                               std::to_string(length));
        }

        const float scale = m_scales[c];
        for (std::size_t i = 0; i < length; ++i) {
            out[begin + i] = window.values[i] * scale;
        }
    }

    return out;
}

Status EmbeddingStore::append(std::span<const float> vector, std::size_t endIndex) {
    if (vector.size() != m_dimension) {
        return Status(invalidArgument("EmbeddingStore::append: expected dimension " +
                                      std::to_string(m_dimension) + ", got " +
                                      std::to_string(vector.size())));
    }
    if (!m_endIndices.empty() && endIndex <= m_endIndices.back()) {
        return Status(dataCorrupt("EmbeddingStore::append: endIndex " +
                                  std::to_string(endIndex) + " is not after the previous " +
                                  std::to_string(m_endIndices.back())));
    }
    m_data.insert(m_data.end(), vector.begin(), vector.end());
    m_endIndices.push_back(endIndex);
    return Status::success();
}

std::size_t EmbeddingStore::lowerBoundRow(std::size_t endIndex) const {
    const auto it = std::lower_bound(m_endIndices.begin(), m_endIndices.end(), endIndex);
    return static_cast<std::size_t>(std::distance(m_endIndices.begin(), it));
}

Status EmbeddingStore::save(const std::string& path) const {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return Status(ioFailure("EmbeddingStore::save: cannot open " + path));
    }
    out.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
    writePod(out, kFormatVersion);
    writePod(out, static_cast<std::uint64_t>(m_dimension));
    writePod(out, static_cast<std::uint64_t>(rows()));
    for (const std::size_t index : m_endIndices) {
        writePod(out, static_cast<std::uint64_t>(index));
    }
    out.write(reinterpret_cast<const char*>(m_data.data()),
              static_cast<std::streamsize>(m_data.size() * sizeof(float)));
    if (!out.good()) {
        return Status(ioFailure("EmbeddingStore::save: write failed for " + path));
    }
    return Status::success();
}

Result<EmbeddingStore> EmbeddingStore::load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return ioFailure("EmbeddingStore::load: cannot open " + path);
    }

    std::array<char, 8> magic{};
    in.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (!in.good() || magic != kMagic) {
        return dataCorrupt("EmbeddingStore::load: bad magic in " + path);
    }

    std::uint32_t version = 0;
    std::uint64_t dimension = 0;
    std::uint64_t rowCount = 0;
    if (!readPod(in, version) || !readPod(in, dimension) || !readPod(in, rowCount)) {
        return dataCorrupt("EmbeddingStore::load: truncated header");
    }
    if (version != kFormatVersion) {
        return dataCorrupt("EmbeddingStore::load: version " + std::to_string(version) +
                           ", expected " + std::to_string(kFormatVersion));
    }
    if (dimension == 0) {
        return dataCorrupt("EmbeddingStore::load: zero dimension");
    }

    EmbeddingStore store(static_cast<std::size_t>(dimension));
    store.m_endIndices.resize(static_cast<std::size_t>(rowCount));
    for (std::size_t i = 0; i < store.m_endIndices.size(); ++i) {
        std::uint64_t value = 0;
        if (!readPod(in, value)) {
            return dataCorrupt("EmbeddingStore::load: truncated index table");
        }
        store.m_endIndices[i] = static_cast<std::size_t>(value);
    }

    store.m_data.resize(static_cast<std::size_t>(rowCount * dimension));
    in.read(reinterpret_cast<char*>(store.m_data.data()),
            static_cast<std::streamsize>(store.m_data.size() * sizeof(float)));
    if (!in.good() && !in.eof()) {
        return dataCorrupt("EmbeddingStore::load: truncated vector data");
    }
    return store;
}

}  // namespace mt::retrieval
