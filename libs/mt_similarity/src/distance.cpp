#include "mt/similarity/distance.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace mt::similarity {
namespace {

constexpr double kEps = 1e-12;

}  // namespace

double l1Shape(std::span<const float> a, std::span<const float> b) {
    const std::size_t n = std::min(a.size(), b.size());
    if (n == 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        sum += std::fabs(static_cast<double>(a[i]) - static_cast<double>(b[i]));
    }
    return sum / static_cast<double>(n);
}

double l2Shape(std::span<const float> a, std::span<const float> b) {
    const std::size_t n = std::min(a.size(), b.size());
    if (n == 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        sum += d * d;
    }
    return std::sqrt(sum / static_cast<double>(n));
}

double correlationDistance(std::span<const float> a, std::span<const float> b) {
    const std::size_t n = std::min(a.size(), b.size());
    if (n < 2) {
        return 1.0;
    }

    double meanA = 0.0;
    double meanB = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        meanA += static_cast<double>(a[i]);
        meanB += static_cast<double>(b[i]);
    }
    meanA /= static_cast<double>(n);
    meanB /= static_cast<double>(n);

    double cov = 0.0;
    double varA = 0.0;
    double varB = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double da = static_cast<double>(a[i]) - meanA;
        const double db = static_cast<double>(b[i]) - meanB;
        cov += da * db;
        varA += da * da;
        varB += db * db;
    }

    if (varA < kEps || varB < kEps) {
        // A constant window has no shape. Reporting maximum distance is the
        // conservative answer: it cannot be claimed to match anything.
        return 1.0;
    }

    const double r = cov / std::sqrt(varA * varB);
    return 0.5 * (1.0 - std::clamp(r, -1.0, 1.0));
}

double cosineDistance(std::span<const float> a, std::span<const float> b) {
    const std::size_t n = std::min(a.size(), b.size());
    if (n == 0) {
        return 1.0;
    }
    double dot = 0.0;
    double normA = 0.0;
    double normB = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double x = static_cast<double>(a[i]);
        const double y = static_cast<double>(b[i]);
        dot += x * y;
        normA += x * x;
        normB += y * y;
    }
    if (normA < kEps || normB < kEps) {
        return 1.0;
    }
    const double cos = dot / std::sqrt(normA * normB);
    return 0.5 * (1.0 - std::clamp(cos, -1.0, 1.0));
}

Result<double> dtwBandDistance(std::span<const float> a, std::span<const float> b,
                               std::size_t band) {
    if (a.empty() || b.empty()) {
        return invalidArgument("dtwBandDistance: empty input");
    }
    const std::size_t n = a.size();
    const std::size_t m = b.size();

    // The band must at least span the length difference or no path exists.
    const std::size_t minBand = (n > m) ? (n - m) : (m - n);
    const std::size_t w = std::max(band, minBand);

    constexpr double kInf = std::numeric_limits<double>::infinity();
    std::vector<double> prev(m + 1, kInf);
    std::vector<double> curr(m + 1, kInf);
    // Path length alongside the cost, so the result can be normalised by the
    // number of steps actually taken rather than by n or m.
    std::vector<double> prevLen(m + 1, 0.0);
    std::vector<double> currLen(m + 1, 0.0);

    prev[0] = 0.0;
    prevLen[0] = 0.0;

    for (std::size_t i = 1; i <= n; ++i) {
        std::fill(curr.begin(), curr.end(), kInf);
        std::fill(currLen.begin(), currLen.end(), 0.0);

        const std::size_t jStart = (i > w) ? (i - w) : 1;
        const std::size_t jEnd = std::min(m, i + w);

        for (std::size_t j = jStart; j <= jEnd; ++j) {
            const double cost =
                std::fabs(static_cast<double>(a[i - 1]) - static_cast<double>(b[j - 1]));

            // Pick the cheapest predecessor: diagonal, up, or left.
            double best = prev[j - 1];
            double bestLen = prevLen[j - 1];
            if (prev[j] < best) {
                best = prev[j];
                bestLen = prevLen[j];
            }
            if (curr[j - 1] < best) {
                best = curr[j - 1];
                bestLen = currLen[j - 1];
            }
            if (best == kInf) {
                continue;
            }
            curr[j] = best + cost;
            currLen[j] = bestLen + 1.0;
        }
        prev.swap(curr);
        prevLen.swap(currLen);
    }

    if (prev[m] == kInf || prevLen[m] <= 0.0) {
        return invalidArgument("dtwBandDistance: no warping path within the band");
    }
    return prev[m] / prevLen[m];
}

Result<double> distance(std::span<const float> a, std::span<const float> b,
                        features::DistanceKind kind, std::size_t dtwBand) {
    if (a.empty() || b.empty()) {
        return invalidArgument("distance: empty input");
    }
    if (a.size() != b.size() && kind != features::DistanceKind::DtwBand) {
        // Equal lengths are a precondition for the point-wise measures.
        // Silently comparing the shorter prefix is how the legacy code ended
        // up comparing different numbers of bars.
        return invalidArgument("distance: length mismatch " + std::to_string(a.size()) + " vs " +
                               std::to_string(b.size()));
    }

    switch (kind) {
        case features::DistanceKind::L1Shape:     return l1Shape(a, b);
        case features::DistanceKind::L2Shape:     return l2Shape(a, b);
        case features::DistanceKind::Correlation: return correlationDistance(a, b);
        case features::DistanceKind::Cosine:      return cosineDistance(a, b);
        case features::DistanceKind::DtwBand:     return dtwBandDistance(a, b, dtwBand);
    }
    return invalidArgument("distance: unhandled distance kind");
}

Result<AlignedDistance> bestShiftDistance(std::span<const float> history,
                                          std::span<const float> current,
                                          features::DistanceKind kind, int maxShift) {
    if (history.empty() || current.empty()) {
        return invalidArgument("bestShiftDistance: empty input");
    }
    if (maxShift < 0) {
        return invalidArgument("bestShiftDistance: maxShift must be >= 0");
    }

    const int n = static_cast<int>(current.size());
    if (maxShift >= n) {
        return invalidArgument("bestShiftDistance: maxShift " + std::to_string(maxShift) +
                               " leaves no overlap in a window of " + std::to_string(n));
    }

    AlignedDistance best;
    best.distance = std::numeric_limits<double>::infinity();

    for (int shift = -maxShift; shift <= maxShift; ++shift) {
        // Overlap of current[i] with history[i + shift] for valid i.
        const int lo = std::max(0, -shift);
        const int hi = std::min(n, static_cast<int>(history.size()) - shift);
        if (hi - lo < 2) {
            continue;
        }
        const auto curSpan = current.subspan(static_cast<std::size_t>(lo),
                                             static_cast<std::size_t>(hi - lo));
        const auto histSpan = history.subspan(static_cast<std::size_t>(lo + shift),
                                              static_cast<std::size_t>(hi - lo));

        Result<double> d = distance(histSpan, curSpan, kind);
        if (!d) {
            return d.error();
        }
        if (d.value() < best.distance) {
            best.distance = d.value();
            best.shift = shift;
        }
    }

    if (!std::isfinite(best.distance)) {
        return invalidArgument("bestShiftDistance: no shift produced a usable overlap");
    }
    return best;
}

}  // namespace mt::similarity
