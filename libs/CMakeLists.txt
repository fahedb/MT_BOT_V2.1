# ---------------------------------------------------------------------------
# libs/ — the refactored engine.
#
# Rules for everything here (docs/REFACTOR_PLAN.md section 5.1):
#   * no singletons, no globals, no I/O inside the maths;
#   * failures are returned as mt::Error, never logged-and-swallowed;
#   * every target compiles at the `strict` warning level;
#   * every target has tests.
#
# Dependency direction is one way, and CMake enforces it: a target may only
# link the ones above it in this file.
# ---------------------------------------------------------------------------

# --- mt_base: Result/Error, logging interface, clock interface -------------
add_library(mt_base INTERFACE)
target_include_directories(mt_base INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/mt_base/include")
target_compile_features(mt_base INTERFACE cxx_std_20)

# --- mt_data: bars, series with validity masks, windows --------------------
add_library(mt_data INTERFACE)
target_include_directories(mt_data INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/mt_data/include")
target_link_libraries(mt_data INTERFACE mt_base)

# --- mt_features: channel registry, standardisation, extractors -----------
add_library(mt_features STATIC
    mt_features/src/standardise.cpp
    mt_features/src/channel.cpp
)
target_include_directories(mt_features PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mt_features/include")
target_link_libraries(mt_features PUBLIC mt_data)
mtbot_set_warnings(mt_features strict)
set_target_properties(mt_features PROPERTIES FOLDER "libs")

# --- mt_similarity: distances, calibration, aggregation -------------------
add_library(mt_similarity STATIC
    mt_similarity/src/distance.cpp
    mt_similarity/src/calibration.cpp
    mt_similarity/src/score.cpp
)
target_include_directories(mt_similarity PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mt_similarity/include")
target_link_libraries(mt_similarity PUBLIC mt_features)
mtbot_set_warnings(mt_similarity strict)
set_target_properties(mt_similarity PROPERTIES FOLDER "libs")

if(0)  # placeholder: targets below are added as their sources land
# --- mt_labels: forward labelling ----------------------------------------
add_library(mt_labels STATIC
    mt_labels/src/label.cpp
)
target_include_directories(mt_labels PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mt_labels/include")
target_link_libraries(mt_labels PUBLIC mt_data)
mtbot_set_warnings(mt_labels strict)
set_target_properties(mt_labels PROPERTIES FOLDER "libs")

# --- mt_aggregate: match set -> signal -----------------------------------
add_library(mt_aggregate STATIC
    mt_aggregate/src/aggregate.cpp
)
target_include_directories(mt_aggregate PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mt_aggregate/include")
target_link_libraries(mt_aggregate PUBLIC mt_labels mt_similarity)
mtbot_set_warnings(mt_aggregate strict)
set_target_properties(mt_aggregate PROPERTIES FOLDER "libs")

# --- mt_retrieval: embeddings, search, dedup, embargo --------------------
add_library(mt_retrieval STATIC
    mt_retrieval/src/embedding.cpp
    mt_retrieval/src/search.cpp
)
target_include_directories(mt_retrieval PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mt_retrieval/include")
target_link_libraries(mt_retrieval PUBLIC mt_similarity)
mtbot_set_warnings(mt_retrieval strict)
set_target_properties(mt_retrieval PROPERTIES FOLDER "libs")

# --- mt_policy: decision rule --------------------------------------------
add_library(mt_policy STATIC
    mt_policy/src/policy.cpp
)
target_include_directories(mt_policy PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/mt_policy/include")
target_link_libraries(mt_policy PUBLIC mt_aggregate)
mtbot_set_warnings(mt_policy strict)
set_target_properties(mt_policy PROPERTIES FOLDER "libs")
endif()
