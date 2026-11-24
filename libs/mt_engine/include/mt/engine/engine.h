// ---------------------------------------------------------------------------
// PatternEngine — the whole pipeline, composed.
//
// This is the replacement for PatternGod plus PatternGodScanner's layers, and
// it implements the stages of docs/REFACTOR_PLAN.md section 7.1 in order:
//
//   stage 0/6  regime prefilter and embargo   (mt_retrieval, mt_labels)
//   stage 1    embed                          (mt_retrieval)
//   stage 2    nearest neighbours             (mt_retrieval)
//   stage 3    exact calibrated rescoring     (mt_similarity)
//   stage 4    alignment refinement           (mt_similarity)
//   stage 5    de-duplicate and aggregate     (mt_aggregate)
//              decide                         (mt_policy)
//
// Properties the legacy engine did not have, and which the tests assert:
//
//  * it can return no signal. Every stage cuts on *quality*, never on a fixed
//    fraction of candidates, so when nothing in history resembles the present
//    the answer is "stand aside" rather than the twenty least-bad matches
//    (finding S7).
//  * nothing accumulates into a shared mutable buffer, so no stage can discard
//    an earlier stage's evidence (finding B1).
//  * the embargo is applied by the index, so no caller can forget it
//    (finding B12).
//  * it is a pure function of (history, config, index): evaluating the same
//    bar twice gives the same answer.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "mt/aggregate/aggregate.h"
#include "mt/base/error.h"
#include "mt/data/bar.h"
#include "mt/features/channel_set.h"
#include "mt/labels/label.h"
#include "mt/policy/policy.h"
#include "mt/retrieval/embedding.h"
#include "mt/retrieval/search.h"
#include "mt/similarity/score.h"

namespace mt::engine {

struct EngineConfig {
    std::size_t windowLength = 48;

    retrieval::SearchConfig search;
    similarity::ScoreConfig score;
    labels::LabelConfig label;
    aggregate::AggregateConfig aggregate;
    policy::PolicyConfig policy;

    // Candidates kept after exact rescoring, before labelling.
    std::size_t rescoreTopK = 50;

    // Quality cut on the calibrated score, in [0, 100]. This is the setting
    // the legacy `threshold` pretended to be: there it was the *fraction* of
    // candidates kept per stage (finding B9), so it could never express "only
    // trade genuinely rare matches".
    double minScore = 70.0;

    // Alignment refinement band, in bars. 0 disables it.
    int maxAlignmentShift = 0;

    // Use the approximate index. False forces the exact reference path, which
    // is what the parity tests compare against.
    bool useApproximateIndex = true;
};

struct EngineDiagnostics {
    std::size_t indexCandidates = 0;   // returned by the index
    std::size_t afterFilters = 0;      // after distance/regime filtering
    std::size_t rescored = 0;          // exactly scored
    std::size_t aboveMinScore = 0;     // passed the quality cut and the gates
    std::size_t labelled = 0;          // had a usable forward label
    std::size_t afterDedup = 0;        // independent observations
};

struct EngineResult {
    aggregate::Signal signal;
    policy::Decision decision;

    // The matches the signal was built from, strongest first. Retained so any
    // decision can be explained after the fact.
    std::vector<aggregate::Match> matches;

    // Per-channel breakdown of the best match, for the same reason.
    similarity::MatchScore bestScore;

    EngineDiagnostics diagnostics;
};

class PatternEngine {
public:
    // Takes ownership of the history so the engine cannot be invalidated by a
    // caller mutating it underneath. Builds the channels, the embeddings and
    // the index up front.
    [[nodiscard]] static Result<std::unique_ptr<PatternEngine>> build(
        data::BarSeries bars, features::ChannelRegistry registry,
        similarity::CalibrationSet calibration, EngineConfig config);

    ~PatternEngine();
    PatternEngine(const PatternEngine&) = delete;
    PatternEngine& operator=(const PatternEngine&) = delete;

    // Evaluates the window ending at `endIndex`. Needs no future data: the
    // embargo guarantees every candidate and its label sit strictly in the
    // past.
    [[nodiscard]] Result<EngineResult> evaluateAt(std::size_t endIndex) const;

    [[nodiscard]] const data::BarSeries& bars() const noexcept;
    [[nodiscard]] const features::ChannelRegistry& registry() const noexcept;
    [[nodiscard]] const EngineConfig& config() const noexcept;
    [[nodiscard]] std::size_t embeddingRows() const noexcept;

    // First bar index at which evaluateAt can produce anything: the window
    // must fit and the embargo must leave some history to search.
    [[nodiscard]] std::size_t firstEvaluableIndex() const noexcept;

private:
    PatternEngine();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mt::engine
