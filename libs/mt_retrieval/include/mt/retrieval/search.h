// ---------------------------------------------------------------------------
// Candidate retrieval — docs/REFACTOR_PLAN.md section 7.
//
// Replaces the full scan plus chain of top-fraction cuts with:
//
//   stage 0  regime prefilter    cheap, exact, removes apples-vs-oranges
//   stage 1  embedding           built once (mt_retrieval/embedding.h)
//   stage 2  nearest neighbours  approximate, top-M
//   stage 6  embargo             enforced in the index filter, not by callers
//
// Stages 3-5 (exact rescoring, alignment refinement, de-duplication) live in
// mt_similarity and mt_aggregate, because they are scoring and aggregation
// concerns rather than retrieval ones.
//
// Two index implementations behind one interface:
//
//   ExactSearchIndex - brute force. The reference. Its results define truth
//                      for the recall gate, and it is what the tests compare
//                      against.
//   HnswSearchIndex  - hnswlib. What production uses.
//
// The recall gate (ANN top-M must contain >= 95% of the exact top-K) is a
// test, not a hope: plan section 7.2 stage 2.
//
// The embargo is the fix for finding B12. The legacy engine had nothing
// preventing a match window from overlapping or abutting the current window,
// so a "historical" match could include the present and its label could
// include the future. Here it is a property of the index filter, so no caller
// can forget it.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "mt/base/error.h"
#include "mt/retrieval/embedding.h"

namespace mt::retrieval {

// Stage 0. Coarse buckets that must match (or be deliberately ignored) before
// two windows are compared at all. Comparing a dead Asian-session range with a
// London breakout is a category error the legacy engine made constantly.
struct RegimeKey {
    std::uint8_t volatilityBand = 0;  // ATR percentile bucket
    std::int8_t trendState = 0;       // sign/strength of the higher timeframe
    std::uint8_t session = 0;         // 0 Asia, 1 London, 2 NY, 3 off-hours

    [[nodiscard]] bool operator==(const RegimeKey& other) const noexcept {
        return volatilityBand == other.volatilityBand && trendState == other.trendState &&
               session == other.session;
    }
};

struct Candidate {
    std::size_t row = 0;        // row in the EmbeddingStore
    std::size_t endIndex = 0;   // bar index the window ends at
    double distance = 0.0;      // embedding-space distance (L2)
};

struct SearchConfig {
    // How many candidates the index returns before exact rescoring. The plan
    // budgets ~2000: large enough for high recall, small enough that exact
    // scoring is cheap (section 7.6).
    std::size_t topM = 2000;

    // Bars that must separate a candidate's window end from the current window
    // end, so neither the candidate's data nor its forward horizon reaches
    // into the present. Use mt::labels::embargoBars().
    std::size_t embargoBars = 0;

    bool requireSameRegime = false;

    // Reject candidates beyond this embedding distance. 0 disables it. This is
    // what lets retrieval return *nothing* when the present has no analogue,
    // instead of always returning its least-bad candidates (finding S7).
    double maxDistance = 0.0;

    // hnswlib build and query parameters.
    std::size_t hnswM = 16;
    std::size_t hnswEfConstruction = 200;
    std::size_t hnswEfSearch = 256;
};

class ISearchIndex {
public:
    virtual ~ISearchIndex() = default;

    // Candidates nearest to `query`, closest first. `maxEndIndex` is the
    // embargo cutoff: only windows ending at or before it may be returned.
    [[nodiscard]] virtual Result<std::vector<Candidate>> search(
        std::span<const float> query, std::size_t topM, std::size_t maxEndIndex) const = 0;

    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual std::string_view name() const = 0;
};

// Brute force over the store. O(rows x dimension) per query, which is what the
// legacy engine effectively did for every channel; here it exists as the
// reference implementation and for tests.
class ExactSearchIndex final : public ISearchIndex {
public:
    explicit ExactSearchIndex(const EmbeddingStore& store) : m_store(&store) {}

    [[nodiscard]] Result<std::vector<Candidate>> search(std::span<const float> query,
                                                        std::size_t topM,
                                                        std::size_t maxEndIndex) const override;

    [[nodiscard]] std::size_t size() const override { return m_store->rows(); }
    [[nodiscard]] std::string_view name() const override { return "exact"; }

private:
    const EmbeddingStore* m_store;
};

// hnswlib-backed approximate index.
class HnswSearchIndex final : public ISearchIndex {
public:
    [[nodiscard]] static Result<std::unique_ptr<HnswSearchIndex>> build(
        const EmbeddingStore& store, const SearchConfig& config);

    ~HnswSearchIndex() override;
    HnswSearchIndex(const HnswSearchIndex&) = delete;
    HnswSearchIndex& operator=(const HnswSearchIndex&) = delete;

    [[nodiscard]] Result<std::vector<Candidate>> search(std::span<const float> query,
                                                        std::size_t topM,
                                                        std::size_t maxEndIndex) const override;

    [[nodiscard]] std::size_t size() const override;
    [[nodiscard]] std::string_view name() const override { return "hnsw"; }

private:
    HnswSearchIndex();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// Applies the configured distance cut and regime filter to an index's output.
// Kept separate from the index so both implementations share it exactly.
[[nodiscard]] std::vector<Candidate> applyFilters(std::span<const Candidate> candidates,
                                                  const SearchConfig& config,
                                                  const RegimeKey* queryRegime,
                                                  std::span<const RegimeKey> candidateRegimes);

// Fraction of `truth` that appears in `found`. The recall gate's measurement.
[[nodiscard]] double recallAgainst(std::span<const Candidate> found,
                                   std::span<const Candidate> truth);

}  // namespace mt::retrieval
