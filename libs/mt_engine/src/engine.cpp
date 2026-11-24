#include "mt/engine/engine.h"

#include <algorithm>
#include <utility>

namespace mt::engine {

struct PatternEngine::Impl {
    data::BarSeries bars;
    features::ChannelRegistry registry;
    features::ChannelSet channels;
    EngineConfig config;

    std::unique_ptr<similarity::Scorer> scorer;
    std::unique_ptr<retrieval::EmbeddingBuilder> embedder;
    retrieval::EmbeddingStore store;
    std::unique_ptr<retrieval::ISearchIndex> index;

    std::size_t embargo = 0;
};

PatternEngine::PatternEngine() : m_impl(std::make_unique<Impl>()) {}
PatternEngine::~PatternEngine() = default;

const data::BarSeries& PatternEngine::bars() const noexcept { return m_impl->bars; }
const features::ChannelRegistry& PatternEngine::registry() const noexcept {
    return m_impl->registry;
}
const EngineConfig& PatternEngine::config() const noexcept { return m_impl->config; }
std::size_t PatternEngine::embeddingRows() const noexcept { return m_impl->store.rows(); }

std::size_t PatternEngine::firstEvaluableIndex() const noexcept {
    // The window must fit, and there must be at least one candidate outside
    // the embargo.
    return m_impl->config.windowLength + m_impl->embargo;
}

Result<std::unique_ptr<PatternEngine>> PatternEngine::build(
    data::BarSeries bars, features::ChannelRegistry registry,
    similarity::CalibrationSet calibration, EngineConfig config) {
    if (config.windowLength < 2) {
        return invalidArgument("PatternEngine::build: windowLength must be >= 2");
    }
    if (config.minScore < 0.0 || config.minScore > 100.0) {
        return invalidArgument("PatternEngine::build: minScore must be in [0, 100]");
    }
    if (config.rescoreTopK == 0) {
        return invalidArgument("PatternEngine::build: rescoreTopK must be > 0");
    }
    if (Status status = bars.validate(); !status.ok()) {
        return status.error();
    }
    if (Status status = policy::validate(config.policy); !status.ok()) {
        return status.error();
    }

    std::unique_ptr<PatternEngine> self(new PatternEngine());
    Impl& impl = *self->m_impl;

    impl.bars = std::move(bars);
    impl.registry = std::move(registry);
    impl.config = config;

    // The embargo and the de-duplication radius must both derive from the
    // label horizon and the window length. Deriving them here rather than
    // trusting the caller is what makes finding B12 structurally impossible.
    impl.embargo = labels::embargoBars(config.label, config.windowLength);
    impl.config.search.embargoBars = impl.embargo;
    impl.config.aggregate.dedupRadius =
        std::max(config.windowLength, config.label.horizonBars);
    impl.config.aggregate.labelKind = config.label.primary;

    Result<features::ChannelSet> channels = features::buildDefaultChannels(impl.bars);
    if (!channels) {
        return channels.error();
    }
    impl.channels = std::move(channels).value();

    if (Status status = impl.channels.validateAgainst(impl.registry); !status.ok()) {
        return status.error();
    }

    Result<similarity::Scorer> scorer =
        similarity::Scorer::create(impl.registry, std::move(calibration), config.score);
    if (!scorer) {
        return scorer.error();
    }
    // The scorer normalised the weights; keep the normalised copy so the
    // embedding is scaled by the same numbers the scorer uses.
    impl.registry = scorer.value().registry();
    impl.scorer = std::make_unique<similarity::Scorer>(std::move(scorer).value());

    Result<retrieval::EmbeddingBuilder> embedder =
        retrieval::EmbeddingBuilder::create(impl.registry, {});
    if (!embedder) {
        return embedder.error();
    }
    impl.embedder =
        std::make_unique<retrieval::EmbeddingBuilder>(std::move(embedder).value());

    // One embedding per window that fits. Built once; this is what replaces
    // rescanning all of history on every bar.
    impl.store = retrieval::EmbeddingStore(impl.embedder->dimension());
    impl.store.reserve(impl.bars.size());
    for (std::size_t endIndex = config.windowLength - 1; endIndex < impl.bars.size();
         ++endIndex) {
        Result<std::vector<features::StandardisedWindow>> windows =
            impl.channels.windowsEndingAt(impl.registry, endIndex, config.windowLength);
        if (!windows) {
            return windows.error();
        }
        Result<std::vector<float>> vector = impl.embedder->build(windows.value());
        if (!vector) {
            // Warm-up: an indicator has no value yet, so this window has no
            // embedding. Skipping is correct; it simply is not searchable.
            continue;
        }
        if (Status status = impl.store.append(vector.value(), endIndex); !status.ok()) {
            return status.error();
        }
    }

    if (impl.store.empty()) {
        return failedPrecondition(
            "PatternEngine::build: no window produced a usable embedding. The history is "
            "shorter than the indicator warm-up plus the window length.");
    }

    if (config.useApproximateIndex) {
        Result<std::unique_ptr<retrieval::HnswSearchIndex>> index =
            retrieval::HnswSearchIndex::build(impl.store, impl.config.search);
        if (!index) {
            return index.error();
        }
        impl.index = std::move(index).value();
    } else {
        impl.index = std::make_unique<retrieval::ExactSearchIndex>(impl.store);
    }

    return self;
}

Result<EngineResult> PatternEngine::evaluateAt(std::size_t endIndex) const {
    const Impl& impl = *m_impl;
    EngineResult result;

    if (endIndex >= impl.bars.size()) {
        return outOfRange("PatternEngine::evaluateAt: index " + std::to_string(endIndex) +
                          " beyond history of " + std::to_string(impl.bars.size()));
    }
    if (endIndex < firstEvaluableIndex()) {
        // Too early for a window plus an embargo. Not an error: it is the
        // normal warm-up state, and the answer is "no signal".
        result.decision.reason = policy::RejectReason::NoSignal;
        return result;
    }

    // --- stage 1: embed the present ---------------------------------------
    Result<std::vector<features::StandardisedWindow>> currentWindows =
        impl.channels.windowsEndingAt(impl.registry, endIndex, impl.config.windowLength);
    if (!currentWindows) {
        return currentWindows.error();
    }
    Result<std::vector<float>> query = impl.embedder->build(currentWindows.value());
    if (!query) {
        // The present itself has an unusable channel, so no comparison is
        // meaningful. Standing aside is the only honest answer.
        result.decision.reason = policy::RejectReason::NoSignal;
        return result;
    }

    // --- stage 2 and 6: search, with the embargo as a hard cutoff ---------
    const std::size_t cutoff = endIndex - impl.embargo;
    Result<std::vector<retrieval::Candidate>> candidates =
        impl.index->search(query.value(), impl.config.search.topM, cutoff);
    if (!candidates) {
        return candidates.error();
    }
    result.diagnostics.indexCandidates = candidates.value().size();

    const std::vector<retrieval::Candidate> filtered =
        retrieval::applyFilters(candidates.value(), impl.config.search, nullptr, {});
    result.diagnostics.afterFilters = filtered.size();

    // --- stage 3 and 4: exact calibrated rescoring ------------------------
    struct Rescored {
        std::size_t endIndex;
        similarity::MatchScore score;
    };
    std::vector<Rescored> rescored;
    rescored.reserve(filtered.size());

    for (const retrieval::Candidate& candidate : filtered) {
        Result<std::vector<features::StandardisedWindow>> historyWindows =
            impl.channels.windowsEndingAt(impl.registry, candidate.endIndex,
                                          impl.config.windowLength);
        if (!historyWindows) {
            return historyWindows.error();
        }
        Result<similarity::MatchScore> score =
            impl.scorer->score(historyWindows.value(), currentWindows.value());
        if (!score) {
            return score.error();
        }
        ++result.diagnostics.rescored;

        if (!score.value().valid()) {
            continue;  // too little usable evidence to score
        }
        // Quality cut and hard gates. Note this is an absolute standard, not a
        // share of the candidates (finding S7).
        if (score.value().total() < impl.config.minScore || !score.value().gatesPassed()) {
            continue;
        }
        rescored.push_back({candidate.endIndex, std::move(score).value()});
    }
    result.diagnostics.aboveMinScore = rescored.size();

    std::sort(rescored.begin(), rescored.end(), [](const Rescored& a, const Rescored& b) {
        if (a.score.total() != b.score.total()) return a.score.total() > b.score.total();
        return a.endIndex < b.endIndex;
    });
    if (rescored.size() > impl.config.rescoreTopK) {
        rescored.resize(impl.config.rescoreTopK);
    }

    if (rescored.empty()) {
        // Nothing in history is similar enough. This is the outcome the legacy
        // engine could not produce.
        result.decision.reason = policy::RejectReason::NoSignal;
        return result;
    }
    result.bestScore = rescored.front().score;

    // --- stage 5: label, aggregate, decide --------------------------------
    std::vector<aggregate::Match> matches;
    matches.reserve(rescored.size());
    for (const Rescored& item : rescored) {
        Result<labels::ForwardLabel> label =
            labels::labelForward(impl.bars, item.endIndex, impl.config.label);
        if (!label) {
            return label.error();
        }
        if (!label.value().valid) {
            continue;
        }
        aggregate::Match match;
        match.endIndex = item.endIndex;
        match.score = item.score.total();
        match.label = std::move(label).value();
        matches.push_back(std::move(match));
    }
    result.diagnostics.labelled = matches.size();

    Result<aggregate::Signal> signal = aggregate::aggregate(matches, impl.config.aggregate);
    if (!signal) {
        return signal.error();
    }
    result.signal = std::move(signal).value();
    result.diagnostics.afterDedup = result.signal.matchCount;
    result.matches = std::move(matches);

    result.decision = policy::decide(result.signal, impl.config.policy);
    return result;
}

}  // namespace mt::engine
