// ---------------------------------------------------------------------------
// Channel registry — docs/REFACTOR_PLAN.md sections 6.1 and 7.7.
//
// A *channel* is one comparable series: (indicator, length, timeframe,
// value-or-slope), or any derived feature. The pre-refactor engine had the
// same idea implicit in a hardcoded quadruple loop over
// IndicatorType x IndicatorLen x BarPeriod x IndicatorValueType
// (BaseScoringEngine.cpp), which meant adding a feature required editing the
// scoring and retrieval code.
//
// Here channels are data. Adding one touches a registry, not an algorithm.
// That is the test of whether the layering is right (plan section 7.7).
//
// Each channel also carries what the scorer needs to know about it: how to
// standardise it, which distance suits it, which correlation cluster it
// belongs to, whether it takes part in the retrieval embedding, and its
// learned weight.
// ---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "mt/base/error.h"
#include "mt/features/standardise.h"

namespace mt::features {

// How a channel's standardised window is compared. Definitions live in
// mt_similarity; the registry only records the choice.
enum class DistanceKind {
    L1Shape,   // mean |a - b|. The default: robust, vectorises, and what the
               // Python research path already used successfully.
    L2Shape,   // root mean square difference; punishes single large deviations
    Correlation,  // (1 - pearson) / 2; pure shape, ignores amplitude entirely
    Cosine,
    DtwBand,   // banded dynamic time warping; rescoring stage only, never the
               // wide scan (plan section 6.3)
};

[[nodiscard]] std::string_view toString(DistanceKind kind) noexcept;
[[nodiscard]] Result<DistanceKind> distanceKindFromString(std::string_view name);

struct ChannelSpec {
    // Stable, human-readable, and the key in weight and calibration
    // artefacts. e.g. "ema.20.M15.value", "rsi.14.M5.level".
    std::string id;

    StandardiseConfig standardise;
    DistanceKind distance = DistanceKind::L1Shape;

    // Channels that measure nearly the same thing share a cluster, and weight
    // is assigned per cluster then split inside it. Without this, fifteen
    // near-identical EMA channels carry fifteen channels' worth of influence
    // (finding S4). Empty means "its own cluster".
    std::string cluster;

    // Whether this channel contributes to the compact retrieval embedding
    // (plan section 7.2 stage 1). Channels left out are still scored exactly
    // in the rescoring stage.
    bool inEmbedding = true;

    // Non-negative. Normalised across the registry before use; a channel at
    // zero is excluded from scoring altogether, which is how Phase 3's weight
    // learning prunes the grid.
    double weight = 1.0;

    // A candidate must reach this calibrated similarity on this channel or it
    // is dropped, regardless of its total. 0 disables the gate
    // (plan section 6.5).
    double gate = 0.0;
};

class ChannelRegistry {
public:
    ChannelRegistry() = default;

    [[nodiscard]] Status add(ChannelSpec spec);

    [[nodiscard]] std::size_t size() const noexcept { return m_specs.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_specs.empty(); }

    [[nodiscard]] const std::vector<ChannelSpec>& specs() const noexcept { return m_specs; }
    [[nodiscard]] const ChannelSpec& operator[](std::size_t i) const { return m_specs[i]; }

    [[nodiscard]] Result<std::size_t> indexOf(const std::string& id) const;
    [[nodiscard]] Result<const ChannelSpec*> find(const std::string& id) const;

    // Sets weights from an id -> weight map, as produced by Phase 3's weight
    // learning. Ids absent from the map keep their current weight; ids in the
    // map that are not registered are reported rather than ignored, because
    // silently dropping a learned weight would misreport the model actually
    // in use.
    [[nodiscard]] Status applyWeights(const std::unordered_map<std::string, double>& weights);

    // Scales weights so they sum to 1, after collapsing clusters: each
    // cluster's total weight is divided equally among its members. Returns the
    // number of channels left with a non-zero weight.
    [[nodiscard]] Result<std::size_t> normaliseWeights();

    [[nodiscard]] std::vector<std::size_t> embeddingChannels() const;
    [[nodiscard]] std::vector<std::size_t> gatedChannels() const;

    // Distinct cluster names, in first-seen order.
    [[nodiscard]] std::vector<std::string> clusters() const;

    // A fingerprint of the registry's shape and settings. Calibration and
    // weight artefacts record it, and loading an artefact whose fingerprint
    // does not match is refused: standardisation changes invalidate a
    // calibration (plan section 6.4).
    [[nodiscard]] std::uint64_t fingerprint() const;

private:
    std::vector<ChannelSpec> m_specs;
    std::unordered_map<std::string, std::size_t> m_byId;
};

// --- ready-made registries ------------------------------------------------

// The pre-refactor grid expressed as channels: 3 indicators x 5 lengths x
// 6 timeframes x {value, slope} = 180. Value channels get RobustZ; slope
// channels get RobustZ too, which is the change from the legacy behaviour of
// comparing slopes raw (finding S5). Clusters group by (indicator, valueType)
// so that the five lengths and six timeframes of one indicator do not
// out-vote everything else (finding S4).
//
// Provided mainly so the new engine can be compared against the old one on
// the same inputs. Production registries should be narrower and learned.
[[nodiscard]] ChannelRegistry legacyGridRegistry(std::size_t paaSegments = 0);

// A compact registry of the features the Python research path found useful
// (plan section 7.7), sized for the retrieval embedding.
[[nodiscard]] ChannelRegistry researchBaselineRegistry(std::size_t paaSegments = 10);

}  // namespace mt::features
