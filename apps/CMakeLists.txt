# ---------------------------------------------------------------------------
# apps/ — the command-line tools.
#
# Both read plain CSV on purpose, so the new engine can be calibrated and
# evaluated on real history before any of the live plumbing is rewired
# (docs/REFACTOR_PLAN.md Phase 6). Exporting from the project's existing
# tables is a one-line query.
# ---------------------------------------------------------------------------

add_library(mtbot_app_common INTERFACE)
target_include_directories(mtbot_app_common INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/common")
target_link_libraries(mtbot_app_common INTERFACE mt_data)

# --- mtcalibrate: builds the calibration artefact (Phase 3) ---------------
add_executable(mtcalibrate mtcalibrate/main.cpp)
target_link_libraries(mtcalibrate PRIVATE mtbot_app_common mt_similarity mt_features)
mtbot_set_warnings(mtcalibrate strict)
set_target_properties(mtcalibrate PROPERTIES FOLDER "apps")

# --- mtbacktest: walk-forward evaluation (Phase 8) ------------------------
add_executable(mtbacktest mtbacktest/main.cpp)
target_link_libraries(mtbacktest PRIVATE mtbot_app_common mt_engine)
mtbot_set_warnings(mtbacktest strict)
set_target_properties(mtbacktest PROPERTIES FOLDER "apps")
