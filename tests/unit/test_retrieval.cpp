// Tests for mt_retrieval: embeddings, the exact reference search, the ANN
// index, the recall gate from plan section 7.2, and the embargo (finding B12).
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <vector>

#include "mt/labels/label.h"
#include "mt/retrieval/embedding.h"
#include "mt/retrieval/search.h"
#include "scoring_fixture.h"

namespace {

using mtbot::testing::SplitMix64;

mt::features::ChannelRegistry makeRegistry(std::size_t channels, std::size_t paaSegments) {
    mt::features::ChannelRegistry registry;
    for (std::size_t i = 0; i < channels; ++i) {
        mt::features::ChannelSpec spec;
        spec.id = "c" + std::to_string(i);
        spec.standardise.paaSegments = paaSegments;
        spec.weight = 1.0;
        (void)registry.add(spec);
    }
    (void)registry.normaliseWeights();
    return registry;
}

std::vector<mt::features::StandardisedWindow> makeWindows(std::size_t channels,
                                                          std::size_t paaSegments,
                                                          SplitMix64& rng) {
    std::vector<mt::features::StandardisedWindow> windows;
    for (std::size_t c = 0; c < channels; ++c) {
        mt::features::StandardisedWindow window;
        window.usable = true;
        window.values.resize(paaSegments);
        for (std::size_t i = 0; i < paaSegments; ++i) {
            window.values[i] = static_cast<float>(rng.gaussian());
        }
        windows.push_back(std::move(window));
    }
    return windows;
}

// A populated store plus the vectors that went into it.
struct StoreFixture {
    mt::features::ChannelRegistry registry;
    mt::retrieval::EmbeddingStore store;
    std::vector<std::vector<float>> vectors;
};

StoreFixture makeStore(std::size_t rows, std::size_t channels = 4,
                       std::size_t paaSegments = 8) {
    StoreFixture fixture;
    fixture.registry = makeRegistry(channels, paaSegments);

    auto builder = mt::retrieval::EmbeddingBuilder::create(fixture.registry, {});
    EXPECT_TRUE(builder) << (builder ? "" : builder.error().describe());

    fixture.store = mt::retrieval::EmbeddingStore(builder.value().dimension());
    SplitMix64 rng(0xC0FFEE);
    for (std::size_t row = 0; row < rows; ++row) {
        const auto windows = makeWindows(channels, paaSegments, rng);
        auto vector = builder.value().build(windows);
        EXPECT_TRUE(vector) << (vector ? "" : vector.error().describe());
        fixture.vectors.push_back(vector.value());
        EXPECT_TRUE(fixture.store.append(vector.value(), row * 2).ok());
    }
    return fixture;
}

}  // namespace

// --- embeddings -----------------------------------------------------------

TEST(Embedding, DimensionIsTheSumOfChannelBlocks) {
    const auto registry = makeRegistry(5, 10);
    const auto builder = mt::retrieval::EmbeddingBuilder::create(registry, {});
    ASSERT_TRUE(builder) << builder.error().describe();
    EXPECT_EQ(builder.value().dimension(), 50u);
    EXPECT_EQ(builder.value().channels().size(), 5u);
}

TEST(Embedding, RefusesChannelsWithoutAFixedLength) {
    // Without PAA a block's length is the window length, which varies by
    // search scale; an embedding must be fixed-size.
    const auto registry = makeRegistry(3, 0);
    const auto builder = mt::retrieval::EmbeddingBuilder::create(registry, {});
    ASSERT_FALSE(builder);
    EXPECT_EQ(builder.error().code(), mt::ErrorCode::InvalidArgument);
}

TEST(Embedding, RefusesToBuildFromAnUnusableChannel) {
    const auto registry = makeRegistry(3, 8);
    const auto builder = mt::retrieval::EmbeddingBuilder::create(registry, {});
    ASSERT_TRUE(builder);

    SplitMix64 rng(1);
    auto windows = makeWindows(3, 8, rng);
    windows[1].usable = false;

    // A hole in the vector would make this window spuriously close to
    // everything, so it is refused rather than zero-filled.
    const auto vector = builder.value().build(windows);
    ASSERT_FALSE(vector);
    EXPECT_EQ(vector.error().code(), mt::ErrorCode::FailedPrecondition);
}

TEST(Embedding, WeightScalingMakesEuclideanDistanceTrackChannelWeights) {
    mt::features::ChannelRegistry registry;
    for (int i = 0; i < 2; ++i) {
        mt::features::ChannelSpec spec;
        spec.id = "c" + std::to_string(i);
        spec.standardise.paaSegments = 4;
        spec.weight = (i == 0) ? 9.0 : 1.0;  // 9:1 before normalisation
        (void)registry.add(spec);
    }
    ASSERT_TRUE(registry.normaliseWeights());

    const auto builder = mt::retrieval::EmbeddingBuilder::create(registry, {});
    ASSERT_TRUE(builder);

    // A unit difference confined to channel 0 must move the embedding further
    // than the same difference confined to channel 1.
    SplitMix64 rng(7);
    auto base = makeWindows(2, 4, rng);
    for (auto& window : base) {
        std::fill(window.values.begin(), window.values.end(), 0.0F);
    }

    auto perturbFirst = base;
    perturbFirst[0].values[0] = 1.0F;
    auto perturbSecond = base;
    perturbSecond[1].values[0] = 1.0F;

    const auto origin = builder.value().build(base);
    const auto first = builder.value().build(perturbFirst);
    const auto second = builder.value().build(perturbSecond);
    ASSERT_TRUE(origin);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    const auto l2 = [](const std::vector<float>& a, const std::vector<float>& b) {
        double sum = 0.0;
        for (std::size_t i = 0; i < a.size(); ++i) {
            const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
            sum += d * d;
        }
        return std::sqrt(sum);
    };

    EXPECT_GT(l2(origin.value(), first.value()), l2(origin.value(), second.value()));
}

TEST(EmbeddingStore, RequiresIncreasingEndIndices) {
    mt::retrieval::EmbeddingStore store(4);
    const std::vector<float> vector(4, 1.0F);
    ASSERT_TRUE(store.append(vector, 10).ok());
    // Out-of-order rows would break the embargo cutoff, which relies on
    // chronological order.
    EXPECT_FALSE(store.append(vector, 10).ok());
    EXPECT_FALSE(store.append(vector, 5).ok());
    EXPECT_TRUE(store.append(vector, 11).ok());
}

TEST(EmbeddingStore, RejectsWrongDimension) {
    mt::retrieval::EmbeddingStore store(4);
    const std::vector<float> wrong(3, 1.0F);
    EXPECT_FALSE(store.append(wrong, 0).ok());
}

TEST(EmbeddingStore, RoundTripsThroughDisk) {
    const auto fixture = makeStore(200);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "mtbot_embeddings_roundtrip.bin";
    ASSERT_TRUE(fixture.store.save(path.string()).ok());

    const auto loaded = mt::retrieval::EmbeddingStore::load(path.string());
    ASSERT_TRUE(loaded) << loaded.error().describe();
    EXPECT_EQ(loaded.value().rows(), fixture.store.rows());
    EXPECT_EQ(loaded.value().dimension(), fixture.store.dimension());
    for (std::size_t row = 0; row < loaded.value().rows(); ++row) {
        EXPECT_EQ(loaded.value().endIndexAt(row), fixture.store.endIndexAt(row));
        const auto a = loaded.value().row(row);
        const auto b = fixture.store.row(row);
        for (std::size_t i = 0; i < a.size(); ++i) {
            EXPECT_FLOAT_EQ(a[i], b[i]);
        }
    }
    std::filesystem::remove(path);
}

// --- exact search ---------------------------------------------------------

TEST(ExactSearch, FindsTheNearestRowsClosestFirst) {
    const auto fixture = makeStore(500);
    const mt::retrieval::ExactSearchIndex index(fixture.store);

    // Query with an exact copy of row 123: it must come back first, at zero
    // distance.
    const auto results = index.search(fixture.vectors[123], 10, 100000);
    ASSERT_TRUE(results) << results.error().describe();
    ASSERT_FALSE(results.value().empty());
    EXPECT_EQ(results.value().front().row, 123u);
    EXPECT_NEAR(results.value().front().distance, 0.0, 1e-5);

    for (std::size_t i = 1; i < results.value().size(); ++i) {
        EXPECT_GE(results.value()[i].distance, results.value()[i - 1].distance);
    }
}

TEST(ExactSearch, HonoursTheEmbargoCutoff) {
    const auto fixture = makeStore(500);
    const mt::retrieval::ExactSearchIndex index(fixture.store);

    // Row 400's endIndex is 800. Querying with it but embargoing everything
    // after endIndex 400 must exclude it and everything later.
    const auto results = index.search(fixture.vectors[400], 50, 400);
    ASSERT_TRUE(results);
    for (const auto& candidate : results.value()) {
        EXPECT_LE(candidate.endIndex, 400u)
            << "an embargoed candidate leaked into the result";
    }
}

TEST(ExactSearch, ReturnsNothingWhenEverythingIsEmbargoed) {
    const auto fixture = makeStore(100);
    const mt::retrieval::ExactSearchIndex index(fixture.store);
    // Every row has endIndex >= 0, and row 0's endIndex is 0, so an embargo
    // before that leaves nothing.
    const auto results = index.search(fixture.vectors[50], 10, 0);
    ASSERT_TRUE(results);
    EXPECT_LE(results.value().size(), 1u);
}

TEST(ExactSearch, RejectsWrongQueryDimension) {
    const auto fixture = makeStore(50);
    const mt::retrieval::ExactSearchIndex index(fixture.store);
    const std::vector<float> wrong(3, 0.0F);
    EXPECT_FALSE(index.search(wrong, 10, 100000));
}

// --- approximate search and the recall gate -------------------------------

TEST(HnswSearch, RecallAgainstExactMeetsTheGate) {
    // Plan section 7.2 stage 2: the ANN top-M must contain at least 95% of the
    // exact top-50. This is the test that makes the retrieval swap safe.
    const auto fixture = makeStore(3000);

    mt::retrieval::SearchConfig config;
    config.topM = 300;
    config.hnswM = 16;
    config.hnswEfConstruction = 200;
    config.hnswEfSearch = 400;

    const auto ann = mt::retrieval::HnswSearchIndex::build(fixture.store, config);
    ASSERT_TRUE(ann) << ann.error().describe();
    const mt::retrieval::ExactSearchIndex exact(fixture.store);

    double totalRecall = 0.0;
    constexpr int kQueries = 20;
    for (int q = 0; q < kQueries; ++q) {
        const std::size_t row = static_cast<std::size_t>(q) * 137 % fixture.vectors.size();
        const auto truth = exact.search(fixture.vectors[row], 50, 1000000);
        const auto found = ann.value()->search(fixture.vectors[row], config.topM, 1000000);
        ASSERT_TRUE(truth) << truth.error().describe();
        ASSERT_TRUE(found) << found.error().describe();
        totalRecall += mt::retrieval::recallAgainst(found.value(), truth.value());
    }

    const double meanRecall = totalRecall / kQueries;
    EXPECT_GE(meanRecall, 0.95) << "ANN recall against exact top-50 was " << meanRecall;
}

TEST(HnswSearch, HonoursTheEmbargoCutoff) {
    const auto fixture = makeStore(1000);
    mt::retrieval::SearchConfig config;
    const auto ann = mt::retrieval::HnswSearchIndex::build(fixture.store, config);
    ASSERT_TRUE(ann) << ann.error().describe();

    const auto results = ann.value()->search(fixture.vectors[900], 100, 500);
    ASSERT_TRUE(results) << results.error().describe();
    EXPECT_FALSE(results.value().empty());
    for (const auto& candidate : results.value()) {
        EXPECT_LE(candidate.endIndex, 500u);
    }
}

TEST(HnswSearch, ReturnsNothingWhenEverythingIsEmbargoed) {
    const auto fixture = makeStore(200);
    mt::retrieval::SearchConfig config;
    const auto ann = mt::retrieval::HnswSearchIndex::build(fixture.store, config);
    ASSERT_TRUE(ann);

    // endIndex values are row * 2, so nothing ends before index 0 except row 0.
    const auto results = ann.value()->search(fixture.vectors[100], 10, 0);
    ASSERT_TRUE(results);
    EXPECT_LE(results.value().size(), 1u);
}

TEST(HnswSearch, FindsAnExactDuplicateFirst) {
    const auto fixture = makeStore(1000);
    mt::retrieval::SearchConfig config;
    const auto ann = mt::retrieval::HnswSearchIndex::build(fixture.store, config);
    ASSERT_TRUE(ann);

    const auto results = ann.value()->search(fixture.vectors[500], 10, 1000000);
    ASSERT_TRUE(results);
    ASSERT_FALSE(results.value().empty());
    EXPECT_EQ(results.value().front().row, 500u);
    EXPECT_NEAR(results.value().front().distance, 0.0, 1e-4);
}

// --- filters --------------------------------------------------------------

TEST(ApplyFilters, DistanceCutLetsRetrievalReturnNothing) {
    // Finding S7: the legacy funnel always returned its least-bad candidates
    // because every stage kept a fixed *fraction*. A distance cut is what
    // allows an honest empty result.
    const std::vector<mt::retrieval::Candidate> candidates = {
        {0, 0, 0.5}, {1, 2, 1.5}, {2, 4, 2.5},
    };

    mt::retrieval::SearchConfig config;
    config.maxDistance = 1.0;
    const auto kept = mt::retrieval::applyFilters(candidates, config, nullptr, {});
    ASSERT_EQ(kept.size(), 1u);
    EXPECT_EQ(kept.front().row, 0u);

    config.maxDistance = 0.1;
    EXPECT_TRUE(mt::retrieval::applyFilters(candidates, config, nullptr, {}).empty());
}

TEST(ApplyFilters, RegimeFilterKeepsOnlyComparableWindows) {
    const std::vector<mt::retrieval::Candidate> candidates = {
        {0, 0, 0.1}, {1, 2, 0.2}, {2, 4, 0.3},
    };
    const std::vector<mt::retrieval::RegimeKey> regimes = {
        {1, 1, 0}, {2, 1, 0}, {1, 1, 0},
    };
    const mt::retrieval::RegimeKey query{1, 1, 0};

    mt::retrieval::SearchConfig config;
    config.requireSameRegime = true;
    const auto kept = mt::retrieval::applyFilters(candidates, config, &query, regimes);
    ASSERT_EQ(kept.size(), 2u);
    EXPECT_EQ(kept[0].row, 0u);
    EXPECT_EQ(kept[1].row, 2u);
}

TEST(ApplyFilters, NoRegimeSuppliedMeansNoRegimeFiltering) {
    const std::vector<mt::retrieval::Candidate> candidates = {{0, 0, 0.1}, {1, 2, 0.2}};
    mt::retrieval::SearchConfig config;
    config.requireSameRegime = true;
    EXPECT_EQ(mt::retrieval::applyFilters(candidates, config, nullptr, {}).size(), 2u);
}

TEST(Recall, IsOneWhenTruthIsFullyContained) {
    const std::vector<mt::retrieval::Candidate> truth = {{1, 0, 0.1}, {2, 0, 0.2}};
    const std::vector<mt::retrieval::Candidate> found = {
        {1, 0, 0.1}, {5, 0, 0.15}, {2, 0, 0.2},
    };
    EXPECT_DOUBLE_EQ(mt::retrieval::recallAgainst(found, truth), 1.0);
}

TEST(Recall, IsProportionalToWhatWasMissed) {
    const std::vector<mt::retrieval::Candidate> truth = {
        {1, 0, 0.1}, {2, 0, 0.2}, {3, 0, 0.3}, {4, 0, 0.4},
    };
    const std::vector<mt::retrieval::Candidate> found = {{1, 0, 0.1}, {3, 0, 0.3}};
    EXPECT_DOUBLE_EQ(mt::retrieval::recallAgainst(found, truth), 0.5);
}

// --- end-to-end embargo property -----------------------------------------

TEST(Retrieval, EmbargoDerivedFromLabelConfigExcludesTheRecentPast) {
    const auto fixture = makeStore(2000);
    const mt::retrieval::ExactSearchIndex index(fixture.store);

    mt::labels::LabelConfig labelConfig;
    labelConfig.horizonBars = 15;
    constexpr std::size_t windowLength = 48;
    const std::size_t embargo = mt::labels::embargoBars(labelConfig, windowLength);

    // The present is the window ending at bar index 3000.
    constexpr std::size_t currentEnd = 3000;
    const auto results =
        index.search(fixture.vectors[1500], 100, currentEnd - embargo);
    ASSERT_TRUE(results) << results.error().describe();

    for (const auto& candidate : results.value()) {
        EXPECT_TRUE(mt::labels::isCandidateAllowed(candidate.endIndex, currentEnd, labelConfig,
                                                   windowLength))
            << "candidate ending at " << candidate.endIndex
            << " is inside the embargo of the window ending at " << currentEnd;
    }
}
