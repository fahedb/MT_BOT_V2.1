#include "mt/aggregate/aggregate.h"

#include <algorithm>
#include <cmath>

namespace mt::aggregate {
namespace {

double matchWeight(double score, const AggregateConfig& config) {
    const double normalised = std::clamp(score / 100.0, 0.0, 1.0);
    switch (config.weightMethod) {
        case WeightMethod::Uniform:
            return 1.0;
        case WeightMethod::PowerLaw:
            return std::pow(normalised, config.gamma);
        case WeightMethod::SoftMax:
            return std::exp(normalised / std::max(config.softMaxTemperature, 1e-6));
    }
    return normalised;
}

}  // namespace

std::vector<Match> deduplicate(std::span<const Match> matches, std::size_t radius) {
    std::vector<Match> sorted(matches.begin(), matches.end());

    // Best first, so the strongest match in each neighbourhood survives.
    // Ties broken by index to keep the result deterministic.
    std::sort(sorted.begin(), sorted.end(), [](const Match& a, const Match& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.endIndex < b.endIndex;
    });

    std::vector<Match> kept;
    kept.reserve(sorted.size());
    for (const Match& candidate : sorted) {
        bool clashes = false;
        for (const Match& chosen : kept) {
            const std::size_t gap = (candidate.endIndex > chosen.endIndex)
                                        ? (candidate.endIndex - chosen.endIndex)
                                        : (chosen.endIndex - candidate.endIndex);
            if (gap <= radius) {
                clashes = true;
                break;
            }
        }
        if (!clashes) {
            kept.push_back(candidate);
        }
    }

    std::sort(kept.begin(), kept.end(),
              [](const Match& a, const Match& b) { return a.endIndex < b.endIndex; });
    return kept;
}

double effectiveSampleSize(std::span<const double> weights) {
    double sum = 0.0;
    double sumSquares = 0.0;
    for (const double w : weights) {
        sum += w;
        sumSquares += w * w;
    }
    if (sumSquares <= 0.0) {
        return 0.0;
    }
    return (sum * sum) / sumSquares;
}

double wilsonLowerBound(double proportion, double sampleSize, double z) {
    if (sampleSize <= 0.0) {
        return 0.0;
    }
    const double p = std::clamp(proportion, 0.0, 1.0);
    const double n = sampleSize;
    const double z2 = z * z;

    const double centre = p + z2 / (2.0 * n);
    const double spread = z * std::sqrt((p * (1.0 - p) / n) + (z2 / (4.0 * n * n)));
    const double denominator = 1.0 + z2 / n;

    return std::clamp((centre - spread) / denominator, 0.0, 1.0);
}

Result<Signal> aggregate(std::span<const Match> matches, const AggregateConfig& config) {
    if (config.gamma < 0.0) {
        return invalidArgument("aggregate: gamma must be >= 0");
    }
    if (config.priorStrength < 0.0) {
        return invalidArgument("aggregate: priorStrength must be >= 0");
    }
    if (config.wilsonZ < 0.0) {
        return invalidArgument("aggregate: wilsonZ must be >= 0");
    }

    Signal signal;
    if (matches.empty()) {
        // An empty match set is a legitimate answer, not a failure: it means
        // nothing in history resembles the present. The caller must stand
        // aside rather than treat it as a neutral signal.
        signal.valid = false;
        return signal;
    }

    const std::vector<Match> unique = deduplicate(matches, config.dedupRadius);
    signal.matchCount = unique.size();

    std::vector<double> weights;
    weights.reserve(unique.size());

    double weightUp = 0.0;
    double weightDown = 0.0;
    double weightNoise = 0.0;
    double weightDirectional = 0.0;
    double scoreWeightSum = 0.0;
    double scoreSum = 0.0;

    for (const Match& match : unique) {
        if (!match.label.valid) {
            continue;  // no usable future; cannot vote
        }
        const double w = matchWeight(match.score, config);
        weights.push_back(w);
        scoreSum += match.score * w;
        scoreWeightSum += w;

        switch (match.label.primaryDirection(config.labelKind)) {
            case labels::Direction::Up:
                weightUp += w;
                weightDirectional += w;
                break;
            case labels::Direction::Down:
                weightDown += w;
                weightDirectional += w;
                break;
            case labels::Direction::Noise:
                weightNoise += w;
                if (!config.ignoreNoiseInVote) {
                    weightDirectional += w;
                }
                break;
        }
    }

    if (weights.empty() || scoreWeightSum <= 0.0) {
        signal.valid = false;
        return signal;
    }

    signal.effectiveSampleSize = effectiveSampleSize(weights);
    signal.meanScore = scoreSum / scoreWeightSum;

    const double totalWeight = weightUp + weightDown + weightNoise;
    signal.noiseRatio = (totalWeight > 0.0) ? (weightNoise / totalWeight) : 1.0;

    if (weightDirectional <= 0.0) {
        // Every usable match said NOISE. Direction is genuinely unknown.
        signal.direction = labels::Direction::Noise;
        signal.probability = 0.5;
        signal.probabilityLowerBound = 0.0;
        signal.valid = true;
        return signal;
    }

    // Shrunk probability of UP. The prior pulls small samples towards 0.5, so
    // three agreeing matches cannot masquerade as certainty (finding S10).
    const double alpha = config.priorStrength;
    const double probabilityUp =
        (weightUp + alpha) / (weightDirectional + 2.0 * alpha);

    signal.direction =
        (probabilityUp >= 0.5) ? labels::Direction::Up : labels::Direction::Down;
    signal.probability =
        (signal.direction == labels::Direction::Up) ? probabilityUp : (1.0 - probabilityUp);

    // The bound is taken at the effective sample size, not the match count.
    signal.probabilityLowerBound =
        wilsonLowerBound(signal.probability, signal.effectiveSampleSize, config.wilsonZ);

    signal.agreement = std::clamp(std::fabs(probabilityUp - 0.5) * 2.0, 0.0, 1.0);

    // Expected move and its dispersion, weighted, in the direction-agnostic
    // sense: the mean of the realised forward moves.
    double moveSum = 0.0;
    double moveWeight = 0.0;
    std::size_t w = 0;
    for (const Match& match : unique) {
        if (!match.label.valid) continue;
        const double weight = weights[w++];
        moveSum += match.label.netReturnPoints * weight;
        moveWeight += weight;
    }
    signal.expectedMovePoints = (moveWeight > 0.0) ? (moveSum / moveWeight) : 0.0;

    double deviationSum = 0.0;
    w = 0;
    for (const Match& match : unique) {
        if (!match.label.valid) continue;
        const double weight = weights[w++];
        deviationSum +=
            std::fabs(match.label.netReturnPoints - signal.expectedMovePoints) * weight;
    }
    signal.moveDispersionPoints = (moveWeight > 0.0) ? (deviationSum / moveWeight) : 0.0;

    // Composite confidence: agreement, evidence volume, geometric quality and
    // cleanliness, each in [0, 1]. Deliberately a product, so a single weak
    // component cannot be averaged away.
    const double evidence =
        signal.effectiveSampleSize / (signal.effectiveSampleSize + 10.0);
    const double quality = std::clamp(signal.meanScore / 100.0, 0.0, 1.0);
    const double cleanliness = 1.0 - std::clamp(signal.noiseRatio, 0.0, 1.0);
    signal.confidence = signal.probabilityLowerBound * evidence * quality * cleanliness;

    signal.valid = true;
    return signal;
}

}  // namespace mt::aggregate
