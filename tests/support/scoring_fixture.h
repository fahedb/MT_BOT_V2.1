// ---------------------------------------------------------------------------
// Fixture for the new scoring system: a small channel registry over synthetic
// series, with a calibration built from random window pairs the same way
// apps/mtcalibrate will build the real one.
//
// Deterministic throughout (splitmix64, fixed seeds), so the acceptance tests
// in section 6.9 of docs/REFACTOR_PLAN.md assert on stable numbers.
// ---------------------------------------------------------------------------
#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "mt/data/series.h"
#include "mt/features/channel.h"
#include "mt/features/standardise.h"
#include "mt/similarity/calibration.h"
#include "mt/similarity/distance.h"
#include "mt/similarity/score.h"

namespace mtbot::testing {

class SplitMix64 {
public:
    explicit SplitMix64(std::uint64_t seed) : m_state(seed) {}

    std::uint64_t next() {
        m_state += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = m_state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

    double unit() { return static_cast<double>(next() >> 11) / 9007199254740992.0; }
    double signedUnit() { return unit() * 2.0 - 1.0; }

    // Box-Muller, so noise tests can speak in sigmas.
    double gaussian() {
        const double u1 = std::max(unit(), 1e-12);
        const double u2 = unit();
        return std::sqrt(-2.0 * std::log(u1)) * std::cos(6.283185307179586 * u2);
    }

private:
    std::uint64_t m_state;
};

inline mt::data::Series makeRandomWalk(const std::string& name, std::size_t length,
                                       std::uint64_t seed, double start, double step) {
    SplitMix64 rng(seed);
    mt::data::Series series(name);
    series.reserve(length);
    double level = start;
    for (std::size_t i = 0; i < length; ++i) {
        level += rng.signedUnit() * step;
        series.push(static_cast<float>(level));
    }
    return series;
}

inline mt::data::Series makeBoundedOscillator(const std::string& name, std::size_t length,
                                              std::uint64_t seed) {
    SplitMix64 rng(seed);
    mt::data::Series series(name);
    series.reserve(length);
    double level = 50.0;
    for (std::size_t i = 0; i < length; ++i) {
        level += rng.signedUnit() * 6.0;
        level = std::clamp(level, 2.0, 98.0);
        series.push(static_cast<float>(level));
    }
    return series;
}

struct ScoringFixture {
    mt::features::ChannelRegistry registry;
    std::unordered_map<std::string, mt::data::Series> series;
    mt::similarity::CalibrationSet calibration;
    std::size_t windowLength = 48;

    // Standardised windows for every channel, for the window ending at
    // `endIndex`. Returns one entry per registered channel, in registry order,
    // which is what Scorer::score expects.
    [[nodiscard]] mt::Result<std::vector<mt::features::StandardisedWindow>> windowsEndingAt(
        std::size_t endIndex) const {
        std::vector<mt::features::StandardisedWindow> out;
        out.reserve(registry.size());
        for (const auto& spec : registry.specs()) {
            const auto it = series.find(spec.id);
            if (it == series.end()) {
                return mt::notFound("fixture: no series for channel " + spec.id);
            }
            mt::Result<mt::data::WindowView> view = it->second.window(endIndex, windowLength);
            if (!view) {
                return view.error();
            }
            mt::Result<mt::features::StandardisedWindow> standardised =
                mt::features::standardise(view.value(), spec.standardise);
            if (!standardised) {
                return standardised.error();
            }
            out.push_back(std::move(standardised).value());
        }
        return out;
    }

    // Standardised windows for a caller-supplied series set, used by the
    // invariance tests to feed a transformed copy through the same path.
    [[nodiscard]] mt::Result<std::vector<mt::features::StandardisedWindow>> windowsFrom(
        const std::unordered_map<std::string, mt::data::Series>& source,
        std::size_t endIndex) const {
        std::vector<mt::features::StandardisedWindow> out;
        out.reserve(registry.size());
        for (const auto& spec : registry.specs()) {
            const auto it = source.find(spec.id);
            if (it == source.end()) {
                return mt::notFound("fixture: no series for channel " + spec.id);
            }
            mt::Result<mt::data::WindowView> view = it->second.window(endIndex, windowLength);
            if (!view) {
                return view.error();
            }
            mt::Result<mt::features::StandardisedWindow> standardised =
                mt::features::standardise(view.value(), spec.standardise);
            if (!standardised) {
                return standardised.error();
            }
            out.push_back(std::move(standardised).value());
        }
        return out;
    }
};

// A four-channel registry over 4000 samples, calibrated from 20,000 random
// non-overlapping window pairs. Small enough to run in milliseconds, large
// enough for the calibration to be meaningful.
inline mt::Result<ScoringFixture> makeScoringFixture(std::size_t paaSegments = 12,
                                                     std::size_t windowLength = 48,
                                                     std::size_t seriesLength = 4000,
                                                     std::size_t calibrationPairs = 20000) {
    ScoringFixture fixture;
    fixture.windowLength = windowLength;

    struct ChannelDef {
        const char* id;
        const char* cluster;
        mt::features::StandardiseMode mode;
        mt::features::DistanceKind distance;
        bool oscillator;
        std::uint64_t seed;
    };

    const ChannelDef defs[] = {
        {"price.close.shape", "price", mt::features::StandardiseMode::RobustZ,
         mt::features::DistanceKind::L1Shape, false, 0xA1},
        {"ema.20.shape", "ema", mt::features::StandardiseMode::RobustZ,
         mt::features::DistanceKind::L1Shape, false, 0xA2},
        {"ema.20.slope.shape", "ema", mt::features::StandardiseMode::RobustZ,
         mt::features::DistanceKind::Correlation, false, 0xA3},
        {"rsi.14.shape", "rsi", mt::features::StandardiseMode::RobustZ,
         mt::features::DistanceKind::L1Shape, true, 0xA4},
    };

    for (const ChannelDef& def : defs) {
        mt::features::ChannelSpec spec;
        spec.id = def.id;
        spec.cluster = def.cluster;
        spec.standardise.mode = def.mode;
        spec.standardise.paaSegments = paaSegments;
        spec.distance = def.distance;
        spec.weight = 1.0;
        if (mt::Status status = fixture.registry.add(spec); !status.ok()) {
            return status.error();
        }
        fixture.series.emplace(
            def.id, def.oscillator
                        ? makeBoundedOscillator(def.id, seriesLength, def.seed)
                        : makeRandomWalk(def.id, seriesLength, def.seed, 1800.0, 0.5));
    }

    // Calibration: exactly the procedure in plan section 6.4, over this
    // fixture's own data.
    fixture.calibration.setFingerprint(fixture.registry.fingerprint());

    mt::similarity::CalibrationSamplingConfig sampling;
    sampling.pairCount = calibrationPairs;
    sampling.bins = 256;  // smaller than production; plenty for the tests
    sampling.minSeparation = windowLength;

    const std::size_t windowCount = seriesLength - windowLength + 1;
    mt::Result<std::vector<std::pair<std::size_t, std::size_t>>> pairs =
        mt::similarity::sampleWindowPairs(windowCount, windowLength, sampling);
    if (!pairs) {
        return pairs.error();
    }

    for (const auto& spec : fixture.registry.specs()) {
        const mt::data::Series& channelSeries = fixture.series.at(spec.id);
        std::vector<float> distances;
        distances.reserve(pairs.value().size());

        for (const auto& [i, j] : pairs.value()) {
            mt::Result<mt::data::WindowView> viewA =
                channelSeries.windowFrom(i, windowLength);
            mt::Result<mt::data::WindowView> viewB =
                channelSeries.windowFrom(j, windowLength);
            if (!viewA || !viewB) {
                continue;
            }
            mt::Result<mt::features::StandardisedWindow> a =
                mt::features::standardise(viewA.value(), spec.standardise);
            mt::Result<mt::features::StandardisedWindow> b =
                mt::features::standardise(viewB.value(), spec.standardise);
            if (!a || !b || !a.value().usable || !b.value().usable) {
                continue;
            }
            mt::Result<double> d = mt::similarity::distance(a.value().values, b.value().values,
                                                            spec.distance);
            if (!d) {
                return d.error();
            }
            distances.push_back(static_cast<float>(d.value()));
        }

        mt::Result<mt::similarity::ChannelCalibration> calibration =
            mt::similarity::ChannelCalibration::fromSamples(std::move(distances), sampling.bins);
        if (!calibration) {
            return calibration.error();
        }
        if (mt::Status status =
                fixture.calibration.set(spec.id, std::move(calibration).value());
            !status.ok()) {
            return status.error();
        }
    }

    return fixture;
}

}  // namespace mtbot::testing
