# ---------------------------------------------------------------------------
# Legacy (pre-refactor) targets.
#
# These compile the ORIGINAL sources with no modifications. Their only job is
# to make today's behaviour buildable and testable so the refactor can be
# measured against it (docs/REFACTOR_PLAN.md, Phase 0 "baseline", Phase 1).
#
# Nothing new should be added here. New code goes into libs/.
# ---------------------------------------------------------------------------

set(MTBOT_LEGACY_ROOT "${CMAKE_SOURCE_DIR}")

# ===========================================================================
# Include directories
#
# The legacy .vcxproj files give every project the union of all module
# directories, and the sources include each other by bare filename
# accordingly. Reproduced here rather than untangled, because Phase 1-2 move
# this code into libs/ with real module boundaries.
# ===========================================================================
set(MTBOT_LEGACY_INCLUDE_DIRS
    "${MTBOT_LEGACY_ROOT}/HTTPCore"
    "${MTBOT_LEGACY_ROOT}/MTCore/CUDA_UNIT"
    "${MTBOT_LEGACY_ROOT}/MTCore/MyRobinHoodMap"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Cache"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Casts"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Compare"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_ComputeServer"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_Configurations"
    "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_CudaCalcs"
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
# isolation until the logger is an interface (finding E9).
#
# NOTE: these same sources are also part of mt_legacy_core below. The two
# libraries must never be linked into one binary. Tests use this one; apps use
# mt_legacy_core.
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
# mt_legacy_core — the whole pre-refactor engine as one static library.
#
# One library rather than one per module on purpose: the modules have cyclic
# header dependencies (ChangeData <-> IndicatorMap <-> TimeLine, Scanner <->
# Statistics <-> TradeManager), so per-module static libs would need link
# groups to resolve. Finding E7/E10. The real boundaries arrive with libs/.
#
# Enable with -DMTBOT_BUILD_LEGACY_FULL=ON.
# ===========================================================================
if(MTBOT_BUILD_LEGACY_FULL)

    set(_mtbot_legacy_module_dirs
        HTTPCore
        MTCore/CUDA_UNIT
        MTCore/MTSimulator_Cache
        MTCore/MTSimulator_Casts
        MTCore/MTSimulator_Compare
        MTCore/MTSimulator_Configurations
        MTCore/MTSimulator_DataBaseConnection
        MTCore/MTSimulator_DataExporter
        MTCore/MTSimulator_DataImporter
        MTCore/MTSimulator_DataLoader
        MTCore/MTSimulator_Logger
        MTCore/MTSimulator_MTThreadPool
        MTCore/MTSimulator_ResourcesManager
        MTCore/MTSimulator_Scanner
        MTCore/MTSimulator_Statistics
        MTCore/MTSimulator_Tables
        MTCore/MTSimulator_TimeLines
        MTCore/MTSimulator_TradeManager
        MTCore/TradeSimulator
    )

    set(_mtbot_legacy_sources "")
    foreach(_dir IN LISTS _mtbot_legacy_module_dirs)
        file(GLOB _srcs CONFIGURE_DEPENDS "${MTBOT_LEGACY_ROOT}/${_dir}/*.cpp")
        list(APPEND _mtbot_legacy_sources ${_srcs})
    endforeach()

    # --- exclusions ---------------------------------------------------------
    # MTChart.cpp needs cairo; handled below, added back when cairo is found.
    list(FILTER _mtbot_legacy_sources EXCLUDE REGEX "MTSimulator_DataExporter/MTChart\\.cpp$")

    # HTTPCore/RequestHandler_ComputeServer.cpp — finding E15. It is in no
    # .vcxproj, so it has never been compiled, and it does not compile:
    # Get_AllConfigs(bool) is defined both inline at
    # RequestHandler_ComputeServer.h:46 and out-of-line at
    # RequestHandler_ComputeServer.cpp:65. Same class of defect as B10.
    # It belongs to the distributed-compute path that Phase 4 retires
    # (plan section 11, question Q4), so it is excluded rather than repaired.
    list(FILTER _mtbot_legacy_sources EXCLUDE REGEX "HTTPCore/RequestHandler_ComputeServer\\.cpp$")

    # MTSimulator_ComputeServer is the same experiment, also left out.

    if(NOT _mtbot_legacy_sources)
        message(FATAL_ERROR "MTBOT_BUILD_LEGACY_FULL=ON but no legacy sources were found")
    endif()

    add_library(mt_legacy_core STATIC ${_mtbot_legacy_sources})
    target_include_directories(mt_legacy_core PUBLIC ${MTBOT_LEGACY_INCLUDE_DIRS})
    mtbot_set_warnings(mt_legacy_core legacy)
    set_target_properties(mt_legacy_core PROPERTIES FOLDER "legacy")

    # The legacy .vcxproj files all set <CharacterSet>Unicode, which MSBuild
    # turns into UNICODE/_UNICODE. That is load-bearing, not cosmetic: sql.h
    # maps SQLDriverConnect and SQLExecDirect to their wide variants only when
    # UNICODE is defined, and DataBaseConnection.cpp / BTables.cpp pass
    # SQLWCHAR* arguments. Without these defines the ODBC calls resolve to the
    # ANSI entry points and the build fails with six C2664s.
    target_compile_definitions(mt_legacy_core PUBLIC UNICODE _UNICODE WIN32)

    # --- dependencies -------------------------------------------------------
    find_package(nlohmann_json CONFIG REQUIRED)
    target_link_libraries(mt_legacy_core PUBLIC nlohmann_json::nlohmann_json)

    if(WIN32)
        # odbc32: the SQL Server access in MTSimulator_DataBaseConnection and
        #         the sql.h/sqlext.h users in Casts / TimeLines / TradeManager.
        # ws2_32, crypt32: the vendored httplib.h.
        target_link_libraries(mt_legacy_core PUBLIC odbc32 ws2_32 crypt32)
    endif()

    # CUDA headers only: CudaMemoryController.h includes <cuda_runtime.h>, so
    # the CPU-side translation units need the include path even when the .cu
    # kernels are not compiled. The kernel symbols stay unresolved in this
    # static library and are supplied by mt_cuda when MTBOT_ENABLE_CUDA=ON.
    find_path(MTBOT_CUDA_INCLUDE_DIR cuda_runtime.h
        HINTS
            "$ENV{CUDA_PATH}/include"
            "$ENV{CUDA_PATH_V12_8}/include"
            "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8/include"
    )
    if(MTBOT_CUDA_INCLUDE_DIR)
        target_include_directories(mt_legacy_core PUBLIC "${MTBOT_CUDA_INCLUDE_DIR}")
        message(STATUS "MT_BOT: CUDA headers at ${MTBOT_CUDA_INCLUDE_DIR}")
    else()
        message(WARNING
            "MT_BOT: cuda_runtime.h not found. The CUDA_UNIT host sources will "
            "fail to compile. Install the CUDA Toolkit or set CUDA_PATH.")
    endif()

    # cairo is a pkg-config port in vcpkg with no CMake config, so look it up
    # by hand. Only MTChart.cpp (pattern PNG export) needs it.
    find_path(MTBOT_CAIRO_INCLUDE_DIR cairo.h PATH_SUFFIXES cairo)
    find_library(MTBOT_CAIRO_LIBRARY NAMES cairo)
    if(MTBOT_CAIRO_INCLUDE_DIR AND MTBOT_CAIRO_LIBRARY)
        target_sources(mt_legacy_core PRIVATE
            "${MTBOT_LEGACY_ROOT}/MTCore/MTSimulator_DataExporter/MTChart.cpp")
        target_include_directories(mt_legacy_core PUBLIC "${MTBOT_CAIRO_INCLUDE_DIR}")
        target_link_libraries(mt_legacy_core PUBLIC "${MTBOT_CAIRO_LIBRARY}")
        message(STATUS "MT_BOT: cairo found, MTChart.cpp included")
    else()
        message(STATUS "MT_BOT: cairo not found, MTChart.cpp excluded (PNG export disabled)")
    endif()

    list(LENGTH _mtbot_legacy_sources _mtbot_legacy_source_count)
    message(STATUS "MT_BOT: mt_legacy_core has ${_mtbot_legacy_source_count} sources")
endif()
