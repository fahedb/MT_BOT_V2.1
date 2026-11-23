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

# --- unit tests ---------------------------------------------------------
add_executable(mt_tests
    unit/test_baseline_v0.cpp
    unit/test_characterisation.cpp
)
target_link_libraries(mt_tests PRIVATE
    mt_test_support
    GTest::gtest
    GTest::gtest_main
)
mtbot_set_warnings(mt_tests strict)
set_target_properties(mt_tests PROPERTIES FOLDER "tests")

include(GoogleTest)
gtest_discover_tests(mt_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "unit"
)
