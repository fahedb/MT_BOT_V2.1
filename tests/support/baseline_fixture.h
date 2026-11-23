// ---------------------------------------------------------------------------
// Deterministic synthetic inputs for the pre-refactor scoring baseline.
//
// Everything here is TEST SCAFFOLDING, not production code. It exists so that
// the current scoring kernels can be fed byte-identical inputs on any machine,
// which is what makes a golden baseline meaningful (docs/REFACTOR_PLAN.md,
// Phase 0 "baseline", and the Phase 3 parity work).
//
// Rules:
//   * no <random> (implementation-defined sequences),
//   * integer-only PRNG with a fixed seed,
//   * no floating-point accumulation in the generator itself,
//   * rolling min/max mirrors TimeLineArrayBuilder::BuildMinMaxArray, i.e.
//     min/max over [i, i + patternLen).
// ---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <vector>

namespace mtbot::testing {

// Window sizes used by every baseline artefact. Changing these invalidates
// the committed golden files; regenerate them and say so in the plan's log.
constexpr int kHistLen = 512;
constexpr int kPatternLen = 32;
constexpr int kOutLen = kHistLen - kPatternLen + 1;  // 481
constexpr float kCompareWeight = 10.0f;              // legacy COMAPRE_WEIGHT

// Numerical Recipes LCG. Deterministic across compilers and platforms.
class Lcg {
public:
    explicit Lcg(std::uint32_t seed) : m_state(seed) {}

    std::uint32_t next() {
        m_state = m_state * 1664525u + 1013904223u;
        return m_state;
    }

    // Uniform in [0, 1) with 24 bits of mantissa, exactly representable.
    float unit() {
        return static_cast<float>(next() >> 8) / 16777216.0f;
    }

    // Uniform in [-1, 1).
    float signedUnit() { return unit() * 2.0f - 1.0f; }

private:
    std::uint32_t m_state;
};

struct Series {
    std::vector<float> values;
    std::vector<float> slopes;
    std::vector<float> minValues;
    std::vector<float> maxValues;
    std::vector<float> minSlopes;
    std::vector<float> maxSlopes;
};

// min/max over [i, i + patternLen), for i in [0, count).
// Deliberately identical to the legacy BuildMinMaxArray so the baseline
// reproduces the production normalisation, bugs included.
inline void fillRollingMinMax(const std::vector<float>& in,
                              int count,
                              int patternLen,
                              std::vector<float>& outMin,
                              std::vector<float>& outMax) {
    outMin.assign(static_cast<std::size_t>(count), 0.0f);
    outMax.assign(static_cast<std::size_t>(count), 0.0f);

    for (int i = 0; i < count; ++i) {
        float lo = in[static_cast<std::size_t>(i)];
        float hi = lo;
        for (int j = 0; j < patternLen; ++j) {
            const float v = in[static_cast<std::size_t>(i + j)];
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
        outMin[static_cast<std::size_t>(i)] = lo;
        outMax[static_cast<std::size_t>(i)] = hi;
    }
}

// A price-like series: bounded random walk around `base`, plus the first
// difference as the "slope" channel. Mirrors how EMA/ZLSMA channels look.
inline Series makePriceLikeSeries(std::uint32_t seed,
                                  int length,
                                  int patternLen,
                                  int minMaxCount,
                                  float base,
                                  float step) {
    Lcg rng(seed);
    Series s;
    s.values.resize(static_cast<std::size_t>(length));

    float level = base;
    for (int i = 0; i < length; ++i) {
        level += rng.signedUnit() * step;
        s.values[static_cast<std::size_t>(i)] = level;
    }

    s.slopes.resize(static_cast<std::size_t>(length));
    s.slopes[0] = 0.0f;
    for (int i = 1; i < length; ++i) {
        s.slopes[static_cast<std::size_t>(i)] =
            s.values[static_cast<std::size_t>(i)] - s.values[static_cast<std::size_t>(i - 1)];
    }

    fillRollingMinMax(s.values, minMaxCount, patternLen, s.minValues, s.maxValues);
    fillRollingMinMax(s.slopes, minMaxCount, patternLen, s.minSlopes, s.maxSlopes);
    return s;
}

// An oscillator-like series in [0, 100] with its first difference, i.e. how
// the RSI channels look. Note the legacy RSI kernel compares these RAW.
inline Series makeOscillatorSeries(std::uint32_t seed,
                                   int length,
                                   int patternLen,
                                   int minMaxCount) {
    Lcg rng(seed);
    Series s;
    s.values.resize(static_cast<std::size_t>(length));

    float level = 50.0f;
    for (int i = 0; i < length; ++i) {
        level += rng.signedUnit() * 6.0f;
        if (level < 2.0f) level = 2.0f;
        if (level > 98.0f) level = 98.0f;
        s.values[static_cast<std::size_t>(i)] = level;
    }

    s.slopes.resize(static_cast<std::size_t>(length));
    s.slopes[0] = 0.0f;
    for (int i = 1; i < length; ++i) {
        s.slopes[static_cast<std::size_t>(i)] =
            s.values[static_cast<std::size_t>(i)] - s.values[static_cast<std::size_t>(i - 1)];
    }

    fillRollingMinMax(s.values, minMaxCount, patternLen, s.minValues, s.maxValues);
    fillRollingMinMax(s.slopes, minMaxCount, patternLen, s.minSlopes, s.maxSlopes);
    return s;
}

// The fixed input set every baseline artefact is computed from.
struct BaselineInputs {
    Series histMa;     // EMA-like history
    Series currMa;     // EMA-like current window
    Series histRsi;    // RSI-like history
    Series currRsi;    // RSI-like current window
    Series histMaY;    // second EMA length, for the dual-indicator kernel
    Series currMaY;
};

inline BaselineInputs makeBaselineInputs() {
    BaselineInputs in;
    // Seeds are arbitrary but frozen. History gets kOutLen min/max entries,
    // the current window gets exactly 1 (as the live engine does).
    in.histMa  = makePriceLikeSeries(0x5EED0001u, kHistLen,    kPatternLen, kOutLen, 1800.0f, 0.45f);
    in.currMa  = makePriceLikeSeries(0x5EED0002u, kPatternLen, kPatternLen, 1,       1812.0f, 0.45f);
    in.histRsi = makeOscillatorSeries(0x5EED0003u, kHistLen,    kPatternLen, kOutLen);
    in.currRsi = makeOscillatorSeries(0x5EED0004u, kPatternLen, kPatternLen, 1);
    in.histMaY = makePriceLikeSeries(0x5EED0005u, kHistLen,    kPatternLen, kOutLen, 1798.0f, 0.30f);
    in.currMaY = makePriceLikeSeries(0x5EED0006u, kPatternLen, kPatternLen, 1,       1810.0f, 0.30f);
    return in;
}

}  // namespace mtbot::testing
