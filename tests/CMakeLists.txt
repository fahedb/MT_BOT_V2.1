# ---------------------------------------------------------------------------
# Test suite (docs/REFACTOR_PLAN.md, Phase 0 finding E4: there were none).
#
#   tests/support/  fixtures and shared compute helpers
#   tests/tools/    mt_gen_baseline, writes the golden files
#   tests/unit/     gtest targets
#   tests/golden/   committed expected values
# ---------------------------------------------------------------------------
find_package(GTest CONFIG REQUIRED)

set(MTBOT_GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/baseline_v0")

# Shared headers for fixtures + the legacy math under test.
add_library(mt_test_support INTERFACE)
target_include_directories(mt_test_support INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/support")
target_link_libraries(mt_test_support INTERFACE mt_legacy_scorecalcs)
target_compile_definitions(mt_test_support INTERFACE
    MTBOT_GOLDEN_DIR="${MTBOT_GOLDEN_DIR}")

# --- baseline generator -------------------------------------------------
add_executable(mt_gen_baseline tools/gen_baseline.cpp)
target_link_libraries(mt_gen_baseline PRIVATE mt_test_support)
mtbot_set_warnings(mt_gen_baseline strict)
set_target_properties(mt_gen_baseline PROPERTIES FOLDER "tests")

include(GoogleTest)

# --- unit tests ---------------------------------------------------------
# Fast: the whole set runs in a few seconds, so it can gate every commit.
add_executable(mt_tests
    unit/test_baseline_v0.cpp
    unit/test_characterisation.cpp
    unit/test_base.cpp
    unit/test_data.cpp
    unit/test_features.cpp
    unit/test_similarity.cpp
    unit/test_scoring_acceptance.cpp
    unit/test_labels.cpp
    unit/test_aggregate.cpp
    unit/test_retrieval.cpp
    unit/test_policy.cpp
)
target_link_libraries(mt_tests PRIVATE
    mt_test_support
    mt_engine       # pulls in mt_policy -> mt_aggregate -> mt_labels, mt_retrieval
    GTest::gtest
    GTest::gtest_main
)
mtbot_set_warnings(mt_tests strict)
set_target_properties(mt_tests PROPERTIES FOLDER "tests")

gtest_discover_tests(mt_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "unit"
)

# --- integration tests --------------------------------------------------
# Separate target because each one builds a full engine - channels,
# calibration, embeddings and an ANN index over thousands of bars - and takes
# seconds rather than milliseconds. Run with:
#     ctest --preset vs2022-debug -L integration
# or exclude them while iterating:
#     ctest --preset vs2022-debug -LE integration
add_executable(mt_integration_tests
    integration/test_engine_end_to_end.cpp
)
target_link_libraries(mt_integration_tests PRIVATE
    mt_test_support
    mt_engine
    GTest::gtest
    GTest::gtest_main
)
mtbot_set_warnings(mt_integration_tests strict)
set_target_properties(mt_integration_tests PROPERTIES FOLDER "tests")

gtest_discover_tests(mt_integration_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "integration"
    TEST_PREFIX "integration."
)
