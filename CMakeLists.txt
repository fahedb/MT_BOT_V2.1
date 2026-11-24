# ---------------------------------------------------------------------------
# MT_BOT_V2.1 — root build
#
# Phase 0 of docs/REFACTOR_PLAN.md: a single CMake tree that builds the
# existing sources UNCHANGED, so that the refactor has a reproducible build
# and a place to hang tests. The legacy .sln / .vcxproj files are left in
# place and keep working until Phase 6.
#
# Quick start:
#   cmake --preset vs2022-debug
#   cmake --build --preset vs2022-debug
#   ctest --preset vs2022-debug
# ---------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)

# --- vcpkg --------------------------------------------------------------
# Picked up automatically from VCPKG_ROOT (or -DMTBOT_VCPKG_ROOT=...) unless
# the caller already supplied a toolchain file. Must run before project().
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED MTBOT_VCPKG_ROOT)
        set(_mtbot_vcpkg "${MTBOT_VCPKG_ROOT}")
    elseif(DEFINED ENV{VCPKG_ROOT})
        set(_mtbot_vcpkg "$ENV{VCPKG_ROOT}")
    elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(_mtbot_vcpkg "C:/vcpkg")
    endif()
    if(DEFINED _mtbot_vcpkg AND EXISTS "${_mtbot_vcpkg}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${_mtbot_vcpkg}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "vcpkg toolchain")
        message(STATUS "MT_BOT: using vcpkg at ${_mtbot_vcpkg}")
    else()
        message(STATUS "MT_BOT: no vcpkg found; targets needing cairo/nlohmann-json will be skipped")
    endif()
endif()

project(MT_BOT
    VERSION 2.1.0
    DESCRIPTION "Forex pattern-similarity engine"
    LANGUAGES CXX)

# --- options ------------------------------------------------------------
option(MTBOT_BUILD_TESTS        "Build the test suite"                        ON)
option(MTBOT_BUILD_LEGACY_FULL  "Attempt the full legacy library tree"        OFF)
option(MTBOT_ENABLE_CUDA        "Build the CUDA scoring kernels"              OFF)
option(MTBOT_WARNINGS_AS_ERRORS "Treat warnings as errors in new code"        OFF)

# --- global settings ----------------------------------------------------
set(CMAKE_CXX_STANDARD 20)              # Phase 0, finding E3 (was C++14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

if(MSVC)
    # /permissive- and conforming preprocessor are prerequisites for the
    # C++20 migration; /utf-8 because several sources carry non-ASCII bytes.
    #
    # Deliberately NOT /MP: with the Visual Studio generator, MSBuild already
    # runs projects in parallel, and /MP multiplies that by spawning N cl.exe
    # per project. On the legacy tree (robin_hood.h plus heavy standard
    # headers) that exhausts the compiler heap - "C1060: compiler is out of
    # heap space" - even on a 64 GB machine. Control parallelism with
    # `cmake --build ... --parallel N` instead.
    add_compile_options(/permissive- /utf-8 /Zc:__cplusplus /Zc:preprocessor)
    # No global Windows-header macros here. _CRT_SECURE_NO_WARNINGS, NOMINMAX
    # and WIN32_LEAN_AND_MEAN are each already #defined by the legacy headers
    # that need them (httplib.h, LOGGER.H, PROCESSDATA.H, DataBaseConnection.h,
    # BaseLayerImporter.h, SelectionNodes.h). Defining them again produced 182
    # C4005 macro-redefinition warnings, all self-inflicted. The pre-refactor
    # .vcxproj files did not define them either, so leaving them out is also
    # the faithful configuration. New code in libs/ declares what it needs.
endif()

include(cmake/MTBotWarnings.cmake)

if(MTBOT_ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        set(CMAKE_CUDA_STANDARD 17)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
        message(STATUS "MT_BOT: CUDA enabled (${CMAKE_CUDA_COMPILER_VERSION})")
    else()
        message(WARNING "MT_BOT: MTBOT_ENABLE_CUDA=ON but no CUDA compiler found; disabling")
        set(MTBOT_ENABLE_CUDA OFF)
    endif()
endif()

# --- targets ------------------------------------------------------------
include(cmake/LegacyTargets.cmake)
add_subdirectory(libs)
add_subdirectory(apps)

if(MTBOT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- summary ------------------------------------------------------------
message(STATUS "")
message(STATUS "MT_BOT ${PROJECT_VERSION} configuration")
message(STATUS "  C++ standard ......... ${CMAKE_CXX_STANDARD}")
message(STATUS "  build tests .......... ${MTBOT_BUILD_TESTS}")
message(STATUS "  legacy full tree ..... ${MTBOT_BUILD_LEGACY_FULL}")
message(STATUS "  CUDA ................. ${MTBOT_ENABLE_CUDA}")
message(STATUS "  warnings as errors ... ${MTBOT_WARNINGS_AS_ERRORS}")
message(STATUS "")
