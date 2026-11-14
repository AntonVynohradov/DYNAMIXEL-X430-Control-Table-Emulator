################################################################################
# DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
#
# Project:     DYNAMIXEL-X430-Control-Table-Emulator
# Description: Extracts the library version from the latest Git tag (vX.Y.Z)
#              and exposes it as X430_VERSION_* compile definitions.
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# References:
#   - MISRA C:2012 Guidelines for the use of the C language in critical systems
#   - CMake Documentation: https://cmake.org/documentation/
#
# Last Updated: October 2026
# SPDX-License-Identifier: Apache-2.0
#
################################################################################

# ==============================================================================
# DEFAULT VERSION
# ==============================================================================

# Version unknown: the source tree has no Git metadata of its own or no vX.Y.Z tag.
# The version comes only from the tag; do not change this value for a release.
set(X430_DEFAULT_VERSION "0.0.0")

# ==============================================================================
# VERSION EXTRACTION
# ==============================================================================

find_package(Git QUIET)

set(X430_GIT_TAG "")
if(GIT_FOUND)
    # Only the library's own repository counts: a copy of the library inside an
    # application's repository must not report the application's tags.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --show-prefix
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
        RESULT_VARIABLE X430_GIT_PREFIX_RESULT
        OUTPUT_VARIABLE X430_GIT_PREFIX
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(X430_GIT_PREFIX_RESULT EQUAL 0 AND "${X430_GIT_PREFIX}" STREQUAL "")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
            RESULT_VARIABLE X430_GIT_DESCRIBE_RESULT
            ERROR_VARIABLE X430_GIT_DESCRIBE_ERROR
            OUTPUT_VARIABLE X430_GIT_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

# ==============================================================================
# VERSION PARSING
# ==============================================================================

string(REGEX MATCH "^[Vv]([0-9]+)\\.([0-9]+)\\.([0-9]+)$" X430_VERSION_MATCH "${X430_GIT_TAG}")

if(NOT X430_VERSION_MATCH)
    message(WARNING "No vX.Y.Z Git tag found (${X430_GIT_DESCRIBE_ERROR}); "
                    "using default version ${X430_DEFAULT_VERSION}")
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" X430_VERSION_MATCH "${X430_DEFAULT_VERSION}")
endif()

# ==============================================================================
# VERSION COMPONENTS
# ==============================================================================

set(X430_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(X430_VERSION_MINOR "${CMAKE_MATCH_2}")
set(X430_VERSION_PATCH "${CMAKE_MATCH_3}")

set(X430_VERSION "${X430_VERSION_MAJOR}.${X430_VERSION_MINOR}.${X430_VERSION_PATCH}")

message(STATUS "X430 library is version: ${X430_VERSION}")

# ==============================================================================
# CONFIGURE VERSION DEFINITIONS
# ==============================================================================

list(APPEND X430_COMPILE_DEFINITIONS
    X430_VERSION_MAJOR=${X430_VERSION_MAJOR}
    X430_VERSION_MINOR=${X430_VERSION_MINOR}
    X430_VERSION_PATCH=${X430_VERSION_PATCH}
    X430_VERSION="${X430_VERSION}"
)
