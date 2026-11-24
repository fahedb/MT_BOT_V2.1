// ---------------------------------------------------------------------------
// Per-channel distances — docs/REFACTOR_PLAN.md section 6.3.
//
// These operate on *standardised* windows (mt_features), so they measure shape
// difference and nothing else.
//
// What they replace: RelativeScoreCalc,
//     1 - |h - c| / max(|h|, |c|)
// which is ratio-based rather than difference-based (finding S1). Its
// consequences, all proven in tests/unit/test_characterisation.cpp: noise at
// 1e-8 scores the same 0.5 as a 100-unit gap; a sign flip scores 0, the same
// as a missing value; and it is not a metric. Every function here is a proper
// distance on the standardised space: zero for identical input, symmetric,
// and monotone in disagreement.
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <span>

#include "mt/base/error.h"
#include "mt/features/channel.h"
#include "mt/features/standardise.h"

namespace mt::similarity {

// Mean absolute difference. The default: robust, trivially vectorised, and the
// measure the Python research path already produced results with.
[[nodiscard]] double l1Shape(std::span<const float> a, std::span<const float> b);

// Root mean square difference. Punishes a single large deviation harder than
// L1, which suits channels where one bad bar should disqualify a match.
[[nodiscard]] double l2Shape(std::span<const float> a, std::span<const float> b);

// (1 - pearson) / 2, in [0, 1]. Scale- and offset-free even after
// standardisation, so it compares pure shape and ignores amplitude. Useful for
// slope channels. Returns 1.0 (maximally distant) when either side is
// constant, since a flat line has no shape to correlate.
[[nodiscard]] double correlationDistance(std::span<const float> a, std::span<const float> b);

// (1 - cosine) / 2, in [0, 1].
[[nodiscard]] double cosineDistance(std::span<const float> a, std::span<const float> b);

// Dynamic time warping restricted to a Sakoe-Chiba band of `band` samples,
// normalised by path length. Tolerates small phase shifts, at O(n * band)
// cost, which is why the plan restricts it to the rescoring stage and forbids
// it in the wide scan (section 6.3).
[[nodiscard]] Result<double> dtwBandDistance(std::span<const float> a, std::span<const float> b,
                                             std::size_t band);

// Dispatch on the channel's configured kind.
[[nodiscard]] Result<double> distance(std::span<const float> a, std::span<const float> b,
                                      features::DistanceKind kind, std::size_t dtwBand = 3);

struct AlignedDistance {
    double distance = 0.0;
    int shift = 0;  // samples the history window was moved by; negative = earlier
};

// Alignment refinement, plan section 7.2 stage 4: try shifts in
// [-maxShift, +maxShift] and keep the best. Compares only the overlapping
// span, and normalises by it, so a larger shift is not rewarded for comparing
// fewer samples.
//
// A persistently non-zero best shift is itself a signal that the window length
// or the anchoring is wrong, which is why the shift is returned and not just
// the distance.
[[nodiscard]] Result<AlignedDistance> bestShiftDistance(std::span<const float> history,
                                                        std::span<const float> current,
                                                        features::DistanceKind kind,
                                                        int maxShift);

}  // namespace mt::similarity
