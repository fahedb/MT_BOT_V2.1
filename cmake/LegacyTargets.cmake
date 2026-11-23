# ---------------------------------------------------------------------------
# Legacy (pre-refactor) targets.
#
# These compile the ORIGINAL sources with no modifications. Their only job is
# to make today's behaviour buildable and testable so the refactor can be
# measured against it (docs/REFACTOR_PLAN.md, Phase 0 "baseline").
#
# Nothing new should be added here. New code goes into libs/ from Phase 1 on.
# ---------------------------------------------------------------------------

set(MTBOT_LEGACY_ROOT "${CMAKE_SOURCE_DIR}")

# ===========================================================================
# mt_legacy_scorecalcs
#
# The self-contained part of the current scoring math: the CPU scan kernels
# (RSI / MA / dual-indicator) and the polynomial comparer. These four
# translation units depend only on each other and the standard library, which
# is what makes a byte-exact baseline possible before any restructuring.
#
# Covers REFACTOR_PLAN sections 3.2 - 3.6 except polynomial CONSTRUCTION:
# PolynomBuilder.cpp calls Logger::getInstance() and cannot be compiled in
# isolation. It is picked up in Phase 1 once the logger is an interface (E9).
# ===========================================================================
add_library(mt_legacy_scorecalcs STATIC
    "${MTBOT_LEGACY_ROOT}/MTCore/CUDA_UNIT/CPUEngine_ScoreCalcs_RSI.cpp"
    "${MTBOT_LEGACY_ROOT}/MTCore/CUDA_UNIT/CPUEngine_ScoreCalcs_MA.cpp"
    "${MTBOT_LEGACY_ROOT}/MTCore/CUDA_UNIT/CPUEngine_ScoreCalcs_TwoInd.cpp"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Compare/PolynomComparer.cpp"
)

target_include_directories(mt_legacy_scorecalcs PUBLIC
    "${MTBOT_LEGACY_ROOT}/MTCore/CUDA_UNIT"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Compare"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_TimeLines"
)

mtbot_set_warnings(mt_legacy_scorecalcs legacy)
set_target_properties(mt_legacy_scorecalcs PROPERTIES FOLDER "legacy")

# ===========================================================================
# Full legacy tree (opt-in, expected to be incomplete)
#
# Enable with -DMTBOT_BUILD_LEGACY_FULL=ON to work through the remaining
# modules. This is a Phase 1 task: the blockers are the singletons, the
# cross-module include soup, and the SQL/ODBC + cairo dependencies, not the
# build system itself. Sources are globbed on purpose here — explicit file
# lists arrive with the new libs/ layout, not for code about to be rewritten.
# ===========================================================================
if(MTBOT_BUILD_LEGACY_FULL)
    set(_mtbot_legacy_include_dirs
        "${MTBOT_LEGACY_ROOT}/HTTPCore"
        "${MTBOT_LEGACY_ROOT}/MTCore/CUDA_UNIT"
        "${MTBOT_LEGACY_ROOT}/MTCore/MyRobinHoodMap"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Cache"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Casts"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Compare"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Configurations"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_DataBaseConnection"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_DataExporter"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_DataImporter"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_DataLoader"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Logger"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_MTThreadPool"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_ResourcesManager"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Scanner"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Statistics"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Tables"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_TimeLines"
        "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_TradeManager"
        "${MTBOT_LEGACY_ROOT}/MTCore/TradeSimulator"
    )

    find_package(nlohmann_json CONFIG QUIET)
    if(NOT nlohmann_json_FOUND)
        message(WARNING
            "MTBOT_BUILD_LEGACY_FULL=ON but nlohmann_json was not found. "
            "Install it (vcpkg install nlohmann-json:x64-windows) or leave the "
            "option OFF.")
    endif()

    # One target per legacy module, in rough dependency order. Each is added
    # as it is made to compile; see docs/REFACTOR_PLAN.md Phase 1.
    function(mtbot_add_legacy_module name dir)
        file(GLOB _srcs CONFIGURE_DEPENDS
            "${MTBOT_LEGACY_ROOT}/${dir}/*.cpp")
        if(NOT _srcs)
            message(WARNING "legacy module ${name}: no sources in ${dir}")
            return()
        endif()
        add_library(${name} STATIC ${_srcs})
        target_include_directories(${name} PUBLIC ${_mtbot_legacy_include_dirs})
        if(nlohmann_json_FOUND)
            target_link_libraries(${name} PUBLIC nlohmann_json::nlohmann_json)
        endif()
        mtbot_set_warnings(${name} legacy)
        set_target_properties(${name} PROPERTIES FOLDER "legacy")
    endfunction()

    mtbot_add_legacy_module(mt_legacy_threadpool MTCore/MTSimulator_MTThreadPool)
    mtbot_add_legacy_module(mt_legacy_casts      MTCore/MTSimulator_Casts)
    # TODO(Phase 1): timelines, cache, compare, statistics, trademanager,
    # tables, data{loader,importer,exporter}, scanner, configurations, httpcore.
    # Each needs the singleton and include-path work described in Phase 1.
endif()
