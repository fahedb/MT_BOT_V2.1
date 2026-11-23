# ---------------------------------------------------------------------------
# mtbot_set_warnings(<target> <level>)
#
#   level = strict  -> full diagnostics, for code written during the refactor.
#                      Honours MTBOT_WARNINGS_AS_ERRORS.
#   level = legacy  -> reduced diagnostics, for pre-refactor sources that are
#                      compiled unchanged. Deliberately permissive so the
#                      Phase 0 build is green; Phase 1 raises these to strict
#                      module by module as the code is cleaned up.
#
# See docs/REFACTOR_PLAN.md finding E3 / Phase 0.
# ---------------------------------------------------------------------------
function(mtbot_set_warnings target level)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "mtbot_set_warnings: '${target}' is not a target")
    endif()

    if(MSVC)
        if(level STREQUAL "strict")
            target_compile_options(${target} PRIVATE /W4)
            if(MTBOT_WARNINGS_AS_ERRORS)
                target_compile_options(${target} PRIVATE /WX)
            endif()
        elseif(level STREQUAL "legacy")
            # /W3 minus the noise that pre-refactor code generates en masse:
            #   4244/4267 narrowing conversions (size_t -> int, double -> float)
            #   4018      signed/unsigned comparison
            #   4996      "deprecated" CRT calls
            target_compile_options(${target} PRIVATE
                /W3 /wd4244 /wd4267 /wd4018 /wd4996)
        else()
            message(FATAL_ERROR "mtbot_set_warnings: unknown level '${level}'")
        endif()
    else()
        if(level STREQUAL "strict")
            target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
            if(MTBOT_WARNINGS_AS_ERRORS)
                target_compile_options(${target} PRIVATE -Werror)
            endif()
        else()
            target_compile_options(${target} PRIVATE -Wall)
        endif()
    endif()
endfunction()
