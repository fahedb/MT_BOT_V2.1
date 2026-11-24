// ---------------------------------------------------------------------------
// Match-set aggregation — docs/REFACTOR_PLAN.md section 6.7.
//
// Turns a set of scored, labelled historical matches into one signal.
//
// The legacy voting (StatisticsDecession, MajorityVote, WeightedVote) counted
// matches and reported percentages. Three problems it had, all fixed here:
//
//  * S10, no sample-size awareness: 20 matches at 70% agreement were treated
//    like 2000 at 70%. The fix is an effective sample size and a Wilson lower
//    bound, so thin evidence cannot produce confidence.
//  * S10, inflated independence: consecutive offsets of one historical event
//    were counted as separate observations, with only a +/-30 bar suppression
//    that was unrelated to either the window length or the horizon. The fix is
//    de-overlapping first, by max(windowLength, horizon).
//  * the geometric quality of a match and the trustworthiness of the match
//    *set* were conflated. They are separate numbers here: a match can look
//    beautiful and still be one of three observations.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "mt/base/error.h"
#include "mt/labels/label.h"

namespace mt::aggregate {

struct Match {
    // Index of the bar where the historical window ends, used for
    // de-overlapping.
    std::size_t endIndex = 0;

    // Calibrated match score in [0, 100] from mt_similarity.
    double score = 0.0;

    labels::ForwardLabel label;
};

enum class WeightMethod {
    PowerLaw,  // (score / 100)^gamma
    SoftMax,   // exp((score / 100) / T)
    Uniform,   // every match counts the same
};

struct AggregateConfig {
    WeightMethod weightMethod = WeightMethod::PowerLaw;
    double gamma = 3.0;
    double softMaxTemperature = 0.1;

    // Matches closer together than this are one observation. Set it to
    // max(windowLength, labelHorizon) - anything less double-counts.
    std::size_t dedupRadius = 48;

    // Beta prior strength for the shrunk probability. 2.0 means a 3-match set
    // cannot report more than ~0.71 even if all three agree.
    double priorStrength = 2.0;

    // z for the Wilson lower bound. 1.645 is one-sided 95%.
    double wilsonZ = 1.645;

    // Exclude NOISE-labelled matches from the direction vote. They still
    // count towards the noise ratio, which is itself a confidence input.
    bool ignoreNoiseInVote = true;

    labels::LabelKind labelKind = labels::LabelKind::FirstTouch;
};

struct Signal {
    labels::Direction direction = labels::Direction::Noise;

    // Weighted, shrunk probability of `direction`.
    double probability = 0.5;

    // Wilson lower bound of `probability` at the effective sample size. This,
    // not `probability`, is what the policy layer decides on.
    double probabilityLowerBound = 0.0;

    double expectedMovePoints = 0.0;
    double moveDispersionPoints = 0.0;  // weighted mean absolute deviation

    // (sum w)^2 / sum w^2. Report this, never the raw match count.
    double effectiveSampleSize = 0.0;

    double agreement = 0.0;   // |p_up - 0.5| * 2, in [0, 1]
    double noiseRatio = 0.0;
    double meanScore = 0.0;

    // Composite of the above, in [0, 1]. A convenience for ranking and
    // logging; the policy layer reads the components, not this.
    double confidence = 0.0;

    std::size_t matchCount = 0;  // after de-overlapping
    bool valid = false;
};

// Keeps the highest-scoring match in each neighbourhood of `radius` bars,
// greedily. Returns matches in chronological order.
[[nodiscard]] std::vector<Match> deduplicate(std::span<const Match> matches,
                                             std::size_t radius);

// Effective sample size of a weight vector.
[[nodiscard]] double effectiveSampleSize(std::span<const double> weights);

// Lower bound of the Wilson score interval for a proportion.
[[nodiscard]] double wilsonLowerBound(double proportion, double sampleSize, double z);

[[nodiscard]] Result<Signal> aggregate(std::span<const Match> matches,
                                       const AggregateConfig& config);

}  // namespace mt::aggregate
