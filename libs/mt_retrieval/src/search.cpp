#include "mt/retrieval/search.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

// hnswlib is vendored third-party code and does not compile warning-free at
// /W4. Suppressing here keeps our own strict flags intact rather than lowering
// them for the whole target.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4701)  // potentially uninitialized local in hnswalg.h
#pragma warning(disable : 4267)  // size_t -> smaller type conversions
#pragma warning(disable : 4244)
#endif
#include "hnswlib/hnswlib.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace mt::retrieval {
namespace {

double squaredDistance(std::span<const float> a, std::span<const float> b) {
    double sum = 0.0;
    const std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) {
        const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        sum += d * d;
    }
    return sum;
}

}  // namespace

Result<std::vector<Candidate>> ExactSearchIndex::search(std::span<const float> query,
                                                        std::size_t topM,
                                                        std::size_t maxEndIndex) const {
    if (query.size() != m_store->dimension()) {
        return invalidArgument("ExactSearchIndex::search: query dimension " +
                               std::to_string(query.size()) + " != store dimension " +
                               std::to_string(m_store->dimension()));
    }
    if (topM == 0) {
        return invalidArgument("ExactSearchIndex::search: topM must be > 0");
    }

    // Max-heap of the best topM so far, so memory stays O(topM) rather than
    // O(rows) even over a decade of minutes.
    using Entry = std::pair<double, std::size_t>;  // (distance, row)
    std::priority_queue<Entry> heap;

    for (std::size_t row = 0; row < m_store->rows(); ++row) {
        if (m_store->endIndexAt(row) > maxEndIndex) {
            // Rows are chronological, so everything after this is also
            // embargoed.
            break;
        }
        const double d2 = squaredDistance(query, m_store->row(row));
        if (heap.size() < topM) {
            heap.emplace(d2, row);
        } else if (d2 < heap.top().first) {
            heap.pop();
            heap.emplace(d2, row);
        }
    }

    std::vector<Candidate> out;
    out.reserve(heap.size());
    while (!heap.empty()) {
        const auto [d2, row] = heap.top();
        heap.pop();
        out.push_back({row, m_store->endIndexAt(row), std::sqrt(d2)});
    }
    std::reverse(out.begin(), out.end());  // closest first
    return out;
}

// --- hnswlib ---------------------------------------------------------------

// Restricts a query to rows at or below a cutoff, which is how the embargo is
// enforced inside the index rather than by the caller (finding B12).
class RowCutoffFilter final : public hnswlib::BaseFilterFunctor {
public:
    explicit RowCutoffFilter(hnswlib::labeltype maxRow) : m_maxRow(maxRow) {}

    bool operator()(hnswlib::labeltype label) override { return label <= m_maxRow; }

private:
    hnswlib::labeltype m_maxRow;
};

struct HnswSearchIndex::Impl {
    std::unique_ptr<hnswlib::L2Space> space;
    std::unique_ptr<hnswlib::HierarchicalNSW<float>> index;
    const EmbeddingStore* store = nullptr;
    std::size_t efSearch = 256;
};

HnswSearchIndex::HnswSearchIndex() : m_impl(std::make_unique<Impl>()) {}
HnswSearchIndex::~HnswSearchIndex() = default;

Result<std::unique_ptr<HnswSearchIndex>> HnswSearchIndex::build(const EmbeddingStore& store,
                                                                const SearchConfig& config) {
    if (store.empty()) {
        return invalidArgument("HnswSearchIndex::build: empty embedding store");
    }
    if (store.dimension() == 0) {
        return invalidArgument("HnswSearchIndex::build: zero-dimension store");
    }

    std::unique_ptr<HnswSearchIndex> self(new HnswSearchIndex());
    self->m_impl->store = &store;
    self->m_impl->efSearch = std::max<std::size_t>(config.hnswEfSearch, 1);

    try {
        self->m_impl->space = std::make_unique<hnswlib::L2Space>(store.dimension());
        self->m_impl->index = std::make_unique<hnswlib::HierarchicalNSW<float>>(
            self->m_impl->space.get(), store.rows(), config.hnswM, config.hnswEfConstruction);

        // Row index is the label, so the embargo cutoff is a label comparison
        // and chronological order is preserved.
        for (std::size_t row = 0; row < store.rows(); ++row) {
            self->m_impl->index->addPoint(store.row(row).data(),
                                          static_cast<hnswlib::labeltype>(row));
        }
        self->m_impl->index->setEf(self->m_impl->efSearch);
    } catch (const std::exception& error) {
        return Error(ErrorCode::Unavailable,
                     std::string("HnswSearchIndex::build: ") + error.what());
    }

    return self;
}

std::size_t HnswSearchIndex::size() const {
    return m_impl->store ? m_impl->store->rows() : 0;
}

Result<std::vector<Candidate>> HnswSearchIndex::search(std::span<const float> query,
                                                       std::size_t topM,
                                                       std::size_t maxEndIndex) const {
    if (!m_impl->index || !m_impl->store) {
        return failedPrecondition("HnswSearchIndex::search: index not built");
    }
    if (query.size() != m_impl->store->dimension()) {
        return invalidArgument("HnswSearchIndex::search: query dimension " +
                               std::to_string(query.size()) + " != store dimension " +
                               std::to_string(m_impl->store->dimension()));
    }
    if (topM == 0) {
        return invalidArgument("HnswSearchIndex::search: topM must be > 0");
    }

    // The embargo cutoff in row terms. Rows are chronological, so this is the
    // last row whose window ends early enough to be usable.
    const std::size_t firstDisallowed = m_impl->store->lowerBoundRow(maxEndIndex + 1);
    if (firstDisallowed == 0) {
        return std::vector<Candidate>{};  // everything is embargoed
    }
    RowCutoffFilter filter(static_cast<hnswlib::labeltype>(firstDisallowed - 1));

    std::vector<Candidate> out;
    try {
        const std::size_t k = std::min(topM, firstDisallowed);
        auto queue = m_impl->index->searchKnn(query.data(), k, &filter);
        out.reserve(queue.size());
        while (!queue.empty()) {
            const auto [d2, label] = queue.top();
            queue.pop();
            const std::size_t row = static_cast<std::size_t>(label);
            out.push_back({row, m_impl->store->endIndexAt(row),
                           std::sqrt(static_cast<double>(d2))});
        }
    } catch (const std::exception& error) {
        return Error(ErrorCode::Unavailable,
                     std::string("HnswSearchIndex::search: ") + error.what());
    }

    std::reverse(out.begin(), out.end());  // hnswlib returns furthest first
    return out;
}

// --- shared filtering ------------------------------------------------------

std::vector<Candidate> applyFilters(std::span<const Candidate> candidates,
                                    const SearchConfig& config, const RegimeKey* queryRegime,
                                    std::span<const RegimeKey> candidateRegimes) {
    std::vector<Candidate> out;
    out.reserve(candidates.size());

    for (const Candidate& candidate : candidates) {
        if (config.maxDistance > 0.0 && candidate.distance > config.maxDistance) {
            continue;
        }
        // Regime filtering only applies when both a query regime and a
        // regime for this candidate are available. A missing regime is not
        // treated as a match: it is simply not filtered on, and the caller
        // decides whether to supply regimes at all.
        const bool canCompareRegime = config.requireSameRegime && queryRegime != nullptr &&
                                      candidate.row < candidateRegimes.size();
        if (canCompareRegime && !(candidateRegimes[candidate.row] == *queryRegime)) {
            continue;
        }
        out.push_back(candidate);
    }
    return out;
}

double recallAgainst(std::span<const Candidate> found, std::span<const Candidate> truth) {
    if (truth.empty()) {
        return 1.0;
    }
    std::unordered_set<std::size_t> foundRows;
    foundRows.reserve(found.size() * 2);
    for (const Candidate& candidate : found) {
        foundRows.insert(candidate.row);
    }

    std::size_t hits = 0;
    for (const Candidate& candidate : truth) {
        if (foundRows.count(candidate.row) != 0) {
            ++hits;
        }
    }
    return static_cast<double>(hits) / static_cast<double>(truth.size());
}

}  // namespace mt::retrieval
