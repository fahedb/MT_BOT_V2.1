// ---------------------------------------------------------------------------
// Regression test: the current scoring math must still reproduce the baseline
// captured at the start of the refactor (docs/REFACTOR_PLAN.md, Phase 0).
//
// When a refactor step is meant to change the numbers, the plan says so and
// the golden files are regenerated with mt_gen_baseline, with a note in the
// progress log. A silent diff here means something moved that should not have.
// ---------------------------------------------------------------------------
#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "baseline_compute.h"

namespace {

// Relative tolerance. The kernels accumulate float in a fixed order, so the
// same compiler is bit-stable, but Debug vs Release vectorisation can differ
// in the last bits; 1e-5 relative is tight enough to catch any real change.
constexpr float kRelTolerance = 1e-5f;

std::vector<float> loadGolden(const std::string& name) {
    const std::filesystem::path file =
        std::filesystem::path(MTBOT_GOLDEN_DIR) / (name + ".txt");
    std::ifstream in(file);
    if (!in) {
        ADD_FAILURE() << "missing golden file: " << file.string()
                      << "\nrun mt_gen_baseline to create it";
        return {};
    }
    std::vector<float> values;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        values.push_back(std::stof(line));
    }
    return values;
}

void expectMatchesGolden(const std::string& name, const std::vector<float>& actual) {
    const std::vector<float> expected = loadGolden(name);
    ASSERT_EQ(expected.size(), actual.size()) << name << ": value count changed";

    for (std::size_t i = 0; i < expected.size(); ++i) {
        const float e = expected[i];
        const float a = actual[i];
        const float scale = std::max(1.0f, std::max(std::fabs(e), std::fabs(a)));
        EXPECT_LE(std::fabs(e - a), kRelTolerance * scale)
            << name << " differs at index " << i << ": golden " << e << ", now " << a;
    }
}

const mtbot::testing::BaselineInputs& inputs() {
    static const mtbot::testing::BaselineInputs in = mtbot::testing::makeBaselineInputs();
    return in;
}

}  // namespace

TEST(BaselineV0, FixtureIsDeterministic) {
    // Guards the fixture itself: if the PRNG or the window sizes change, every
    // other baseline comparison becomes meaningless, so fail loudly here first.
    const auto a = mtbot::testing::makeBaselineInputs();
    const auto b = mtbot::testing::makeBaselineInputs();
    ASSERT_EQ(a.histMa.values.size(), static_cast<std::size_t>(mtbot::testing::kHistLen));
    ASSERT_EQ(a.currMa.values.size(), static_cast<std::size_t>(mtbot::testing::kPatternLen));
    ASSERT_EQ(a.histMa.minValues.size(), static_cast<std::size_t>(mtbot::testing::kOutLen));
    ASSERT_EQ(a.currMa.minValues.size(), 1u);
    for (std::size_t i = 0; i < a.histMa.values.size(); ++i) {
        ASSERT_FLOAT_EQ(a.histMa.values[i], b.histMa.values[i]) << "at " << i;
        ASSERT_FLOAT_EQ(a.histRsi.values[i], b.histRsi.values[i]) << "at " << i;
    }
}

TEST(BaselineV0, RsiFullScan) {
    expectMatchesGolden("rsi_fullscan", mtbot::testing::computeRsiFullScan(inputs()));
}

TEST(BaselineV0, MaFullScan) {
    expectMatchesGolden("ma_fullscan", mtbot::testing::computeMaFullScan(inputs()));
}

TEST(BaselineV0, TwoIndFullScan) {
    expectMatchesGolden("twoind_fullscan", mtbot::testing::computeTwoIndFullScan(inputs()));
}

TEST(BaselineV0, RelativeScoreGrid) {
    expectMatchesGolden("relative_score", mtbot::testing::computeRelativeScoreGrid());
}

TEST(BaselineV0, PolynomCompareProbes) {
    expectMatchesGolden("polynom_compare", mtbot::testing::computePolynomCompareProbes());
}

TEST(BaselineV0, ScanProducesNonTrivialScores) {
    // Sanity: a baseline of all zeros would pass the golden comparison while
    // proving nothing, so assert the kernels actually did work.
    const auto rsi = mtbot::testing::computeRsiFullScan(inputs());
    const auto ma = mtbot::testing::computeMaFullScan(inputs());

    const auto allZero = [](const std::vector<float>& v) {
        for (const float x : v) {
            if (x != 0.0f) return false;
        }
        return true;
    };

    EXPECT_FALSE(allZero(rsi));
    EXPECT_FALSE(allZero(ma));

    // Documented upper bound from REFACTOR_PLAN section 3.3:
    // weight * 2 * patternLen is the perfect-match score for one channel.
    const float maxPossible =
        mtbot::testing::kCompareWeight * 2.0f * static_cast<float>(mtbot::testing::kPatternLen);
    for (const float x : rsi) {
        EXPECT_GE(x, 0.0f);
        EXPECT_LE(x, maxPossible + 1e-3f);
    }
}
