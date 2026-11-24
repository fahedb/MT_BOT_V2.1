// ---------------------------------------------------------------------------
// Window embeddings — docs/REFACTOR_PLAN.md section 7.2 stage 1.
//
// One compact float vector per historical window, so candidate retrieval stops
// being a full scan of every channel over all of history.
//
// The legacy engine computed the full 180-channel score for every offset in
// history on every bar - O(H x L x 180) - which is why it needed CUDA and a
// distributed compute unit to keep up, and why it still had to truncate
// history. The embedding reduces that to one nearest-neighbour query plus an
// exact rescoring of a few thousand survivors (section 7.6).
//
// Channels are pre-multiplied by sqrt(w_c) so that Euclidean distance in
// embedding space approximates the weighted channel distance, which means the
// index ranks by something close to the final score rather than by an
// unrelated proxy.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "mt/base/error.h"
#include "mt/features/channel.h"
#include "mt/features/standardise.h"

namespace mt::retrieval {

struct EmbeddingConfig {
    // Pre-multiply each channel's block by sqrt(weight), so squared Euclidean
    // distance in embedding space equals the weighted sum of squared channel
    // differences.
    bool scaleByWeight = true;
};

class EmbeddingBuilder {
public:
    // Fails when no channel participates, or when a participating channel has
    // no fixed length (PAA disabled), since the embedding must be a fixed-size
    // vector for every window.
    [[nodiscard]] static Result<EmbeddingBuilder> create(
        const features::ChannelRegistry& registry, EmbeddingConfig config);

    [[nodiscard]] std::size_t dimension() const noexcept { return m_dimension; }

    // Indices of the registry channels that make up the embedding, in order.
    [[nodiscard]] const std::vector<std::size_t>& channels() const noexcept {
        return m_channels;
    }

    // `windows` is indexed by registry channel, as Scorer::score expects. Any
    // participating channel being unusable makes the whole embedding
    // unusable: a vector with a hole in it would be silently closer to
    // everything.
    [[nodiscard]] Result<std::vector<float>> build(
        std::span<const features::StandardisedWindow> windows) const;

private:
    EmbeddingBuilder() = default;

    std::vector<std::size_t> m_channels;
    std::vector<std::size_t> m_offsets;  // start of each channel's block
    std::vector<float> m_scales;         // sqrt(weight) per channel
    std::size_t m_dimension = 0;
};

// Append-only matrix of embeddings plus the bar index each row describes.
// Rows are in chronological order, which is what lets the embargo filter be a
// simple row cutoff.
class EmbeddingStore {
public:
    EmbeddingStore() = default;
    explicit EmbeddingStore(std::size_t dimension) : m_dimension(dimension) {}

    [[nodiscard]] std::size_t dimension() const noexcept { return m_dimension; }
    [[nodiscard]] std::size_t rows() const noexcept { return m_endIndices.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_endIndices.empty(); }

    void reserve(std::size_t rows) {
        m_data.reserve(rows * m_dimension);
        m_endIndices.reserve(rows);
    }

    // `endIndex` must increase, mirroring BarSeries: out-of-order rows would
    // break the embargo cutoff.
    [[nodiscard]] Status append(std::span<const float> vector, std::size_t endIndex);

    [[nodiscard]] std::span<const float> row(std::size_t index) const {
        return std::span<const float>(m_data).subspan(index * m_dimension, m_dimension);
    }

    [[nodiscard]] std::size_t endIndexAt(std::size_t index) const {
        return m_endIndices[index];
    }

    [[nodiscard]] const std::vector<std::size_t>& endIndices() const noexcept {
        return m_endIndices;
    }

    [[nodiscard]] const float* data() const noexcept { return m_data.data(); }

    // The first row whose endIndex is >= `endIndex`; rows() when none is.
    // Used to turn an embargo expressed in bars into a row cutoff.
    [[nodiscard]] std::size_t lowerBoundRow(std::size_t endIndex) const;

    // Binary format: magic, version, dimension, row count, end indices, data.
    [[nodiscard]] Status save(const std::string& path) const;
    [[nodiscard]] static Result<EmbeddingStore> load(const std::string& path);

    static constexpr std::uint32_t kFormatVersion = 1;

private:
    std::size_t m_dimension = 0;
    std::vector<float> m_data;  // row-major, rows() x dimension()
    std::vector<std::size_t> m_endIndices;
};

}  // namespace mt::retrieval
