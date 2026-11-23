// ---------------------------------------------------------------------------
// Runs the PRE-REFACTOR scoring math over the frozen fixture inputs.
//
// Shared by the generator (tools/gen_baseline.cpp) and the regression test
// (unit/test_baseline_v0.cpp) so that both call the production functions in
// exactly the same way. Nothing in here may be "improved" — it is the record
// of what the engine computed before the refactor.
// ---------------------------------------------------------------------------
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "baseline_fixture.h"

// Legacy production headers, included unchanged.
#include "CPUEngine_ScoreCalcs.h"               // FullScan_CPU_MA / _RSI declarations
#include "CPUEngine_ScoreCalcs_RSI.h"
#include "CPUEngine_ScoreCalcs_MA.h"
#include "CPUEngine_ScoreCalcs_TwoInd.h"
#include "KernelEngine_ScoreCalcs_General.h"    // RelativeScoreCalc, pair_score
#include "PolynomComparer.h"
#include "Polynom.h"

namespace mtbot::testing {

// name -> values. Written one value per line, compared element-wise.
using BaselineArtefact = std::pair<std::string, std::vector<float>>;

// --- 1. RSI single-indicator full scan ---------------------------------
inline std::vector<float> computeRsiFullScan(const BaselineInputs& in) {
    std::vector<float> scores(static_cast<std::size_t>(kOutLen), 0.0f);
    FullScan_CPU_RSI(in.histRsi.values.data(),
                     in.histRsi.slopes.data(),
                     in.currRsi.values.data(),
                     in.currRsi.slopes.data(),
                     scores.data(),
                     0, kOutLen,
                     kPatternLen, kOutLen, kCompareWeight);
    return scores;
}

// --- 2. MA single-indicator full scan ----------------------------------
inline std::vector<float> computeMaFullScan(const BaselineInputs& in) {
    std::vector<float> scores(static_cast<std::size_t>(kOutLen), 0.0f);
    FullScan_CPU_MA(in.histMa.values.data(),
                    in.histMa.slopes.data(),
                    in.histMa.minValues.data(),
                    in.histMa.maxValues.data(),
                    in.histMa.minSlopes.data(),
                    in.histMa.maxSlopes.data(),
                    in.currMa.values.data(),
                    in.currMa.slopes.data(),
                    in.currMa.minValues.data(),
                    in.currMa.maxValues.data(),
                    in.currMa.minSlopes.data(),
                    in.currMa.maxSlopes.data(),
                    scores.data(),
                    0, kOutLen,
                    kPatternLen, kOutLen, kCompareWeight);
    return scores;
}

// --- 3. Dual-indicator ratio full scan ---------------------------------
inline std::vector<float> computeTwoIndFullScan(const BaselineInputs& in) {
    std::vector<float> scores(static_cast<std::size_t>(kOutLen), 0.0f);
    FullScan_CPU_TwoInd(in.histMa.values.data(),
                        in.histMa.slopes.data(),
                        in.histMa.minValues.data(),
                        in.histMa.maxValues.data(),
                        in.histMa.minSlopes.data(),
                        in.histMa.maxSlopes.data(),

                        in.currMa.values.data(),
                        in.currMa.slopes.data(),
                        in.currMa.minValues.data(),
                        in.currMa.maxValues.data(),
                        in.currMa.minSlopes.data(),
                        in.currMa.maxSlopes.data(),

                        in.histMaY.values.data(),
                        in.histMaY.slopes.data(),
                        in.histMaY.minValues.data(),
                        in.histMaY.maxValues.data(),
                        in.histMaY.minSlopes.data(),
                        in.histMaY.maxSlopes.data(),

                        in.currMaY.values.data(),
                        in.currMaY.slopes.data(),
                        in.currMaY.minValues.data(),
                        in.currMaY.maxValues.data(),
                        in.currMaY.minSlopes.data(),
                        in.currMaY.maxSlopes.data(),

                        0, kOutLen,
                        scores.data(),
                        kPatternLen, kOutLen, kCompareWeight);
    return scores;
}

// --- 4. RelativeScoreCalc probe grid -----------------------------------
// A fixed grid of (hist, curr) pairs spanning the interesting regions:
// equal values, near-zero magnitudes, sign flips, large ratios.
inline std::vector<std::pair<float, float>> relativeScoreProbes() {
    return {
        {0.0f, 0.0f},      {1.0f, 1.0f},        {1.0f, 2.0f},
        {2.0f, 1.0f},      {-1.0f, 1.0f},       {1.0f, -1.0f},
        {0.001f, 0.002f},  {0.0001f, 0.0002f},  {1e-8f, 2e-8f},
        {100.0f, 101.0f},  {50.0f, 55.0f},      {0.0f, 1.0f},
        {1.0f, 0.0f},      {-5.0f, -5.5f},      {1000.0f, 1.0f},
        {0.5f, 0.5000001f},
    };
}

inline std::vector<float> computeRelativeScoreGrid() {
    std::vector<float> out;
    for (const auto& [h, c] : relativeScoreProbes()) {
        out.push_back(RelativeScoreCalc(h, c));
    }
    return out;
}

// --- 5. Polynomial comparer probes -------------------------------------
// Eight frozen polynomial pairs, degree 4. For each pair the three legacy
// comparison metrics are recorded in order: eval, rmse, weighted-coeffs.
inline std::vector<std::pair<Polynom, Polynom>> polynomProbePairs() {
    Lcg rng(0x5EED0100u);
    std::vector<std::pair<Polynom, Polynom>> pairs;
    for (int p = 0; p < 8; ++p) {
        std::vector<double> a(5), b(5);
        for (int i = 0; i < 5; ++i) {
            a[static_cast<std::size_t>(i)] = static_cast<double>(rng.signedUnit()) * 2.0;
        }
        for (int i = 0; i < 5; ++i) {
            // Second polynomial is a perturbation of the first, so the metrics
            // land in a useful range rather than all near zero.
            b[static_cast<std::size_t>(i)] =
                a[static_cast<std::size_t>(i)] + static_cast<double>(rng.signedUnit()) * 0.3;
        }
        Polynom pa;
        Polynom pb;
        pa.SetPolynom(a);
        pb.SetPolynom(b);
        pairs.emplace_back(std::move(pa), std::move(pb));
    }
    return pairs;
}

inline std::vector<float> computePolynomCompareProbes() {
    std::vector<float> out;
    for (const auto& [pa, pb] : polynomProbePairs()) {
        out.push_back(static_cast<float>(PolynomComparer::Compare_ByEval(pa, pb)));
        out.push_back(static_cast<float>(PolynomComparer::Compare_ByRMSE(pa, pb)));
        out.push_back(static_cast<float>(PolynomComparer::Compare_ByWeightedCoefficients(pa, pb)));
    }
    return out;
}

// --- all artefacts -----------------------------------------------------
inline std::vector<BaselineArtefact> computeAllBaselineArtefacts() {
    const BaselineInputs in = makeBaselineInputs();
    return {
        {"rsi_fullscan",     computeRsiFullScan(in)},
        {"ma_fullscan",      computeMaFullScan(in)},
        {"twoind_fullscan",  computeTwoIndFullScan(in)},
        {"relative_score",   computeRelativeScoreGrid()},
        {"polynom_compare",  computePolynomCompareProbes()},
    };
}

}  // namespace mtbot::testing
