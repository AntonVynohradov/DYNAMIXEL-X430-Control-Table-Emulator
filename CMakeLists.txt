################################################################################
# DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
#
# Project:     DYNAMIXEL-X430-Control-Table-Emulator
# Description: Top-level CMake file: compiler settings, library, tests,
#              examples and static analysis targets.
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
# CMAKE CONFIGURATION
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

# cmake/ is not put on CMAKE_MODULE_PATH: DXL2-C includes its own version.cmake by
# name, so the modules of this library are included by absolute path.

# ==============================================================================
# PROJECT CONFIGURATION
# ==============================================================================

project(X430
    DESCRIPTION "DYNAMIXEL X430 Control Table Emulator"
    LANGUAGES C
    )

if(CMAKE_HOST_WIN32)
    set(CMAKE_COLOR_MAKEFILE OFF)
endif()

# ==============================================================================
# COMPILER FLAGS & OPTIONS
# ==============================================================================

# C11 is the newest language revision covered by MISRA C:2012 (Amendment 2)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler warning flags
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -Wconversion -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -Wconversion -O2 -DNDEBUG")

# ==============================================================================
# LIBRARY/EXECUTABLE TARGETS
# ==============================================================================

# Unit tests and the host example are optional and enabled with CMake options
option(UNIT_TESTS "Build the unit tests (GoogleTest) of the libraries" OFF)
option(X430_EXAMPLES "Build the host example (serial/pty simulator, POSIX)" OFF)

# The static library target (and DXL2-C) is defined in x430.cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/x430.cmake)

# ==============================================================================
# UNIT TESTS
# ==============================================================================

if(UNIT_TESTS)
    # GTest integration, test discovery and the coverage target
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/unit_tests.cmake)

    # Model data against the ROBOTIS e-Manual tables (Python, standard library only)
    add_test(NAME x430_models
        COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_SOURCE_DIR}/tests/models -v
    )
    set_tests_properties(x430_models PROPERTIES LABELS "models"
        ENVIRONMENT "X430_ROOT=${CMAKE_SOURCE_DIR}")
endif()

# ==============================================================================
# EXAMPLES
# ==============================================================================

if(X430_EXAMPLES)
    enable_testing()
    add_subdirectory(examples)
endif()


# ==============================================================================
# HEADERS OF THE ANALYSIS AND OF THE DOCUMENTATION SNIPPETS
# ==============================================================================

# The sources include the generated model header and DXL2-C: one folder with copies
# of them is the configuration folder (CONFIG_DIR) of the MISRA analysis and an
# include folder of tools/docs/check_snippets.py.
#
# No file(GLOB) here: the build folders of the presets are named build/X430[...], and
# GLOB reads the brackets as a character class (it then finds nothing).
set(X430_MISRA_INCLUDE_DIR ${CMAKE_BINARY_DIR}/x430/analysis-include)
file(COPY ${X430_GENERATED_DIR}/x430_model_info.h ${dxl2_SOURCE_DIR}/dxl2.h
     DESTINATION ${X430_MISRA_INCLUDE_DIR})
file(COPY ${dxl2_SOURCE_DIR}/inc/ DESTINATION ${X430_MISRA_INCLUDE_DIR}
     FILES_MATCHING PATTERN "*.h")
foreach(X430_REQUIRED_HEADER x430_model_info.h dxl2.h dxl2_config.h dxl2_link.h)
    if(NOT EXISTS ${X430_MISRA_INCLUDE_DIR}/${X430_REQUIRED_HEADER})
        message(FATAL_ERROR "x430: ${X430_REQUIRED_HEADER} missing in ${X430_MISRA_INCLUDE_DIR}")
    endif()
endforeach()

# ==============================================================================
# STATIC ANALYSIS TARGETS
# ==============================================================================

# Cppcheck static analysis target
# Runs Cppcheck with all checks enabled and writes the result to cppcheck.txt in
# the build directory and to the console. Paths listed in 'cppcheck-excludes.txt'
# are excluded. The target exists only if Cppcheck is found on the system.

find_program(CPPCHECK_EXECUTABLE cppcheck)
if(CPPCHECK_EXECUTABLE)


    set(CPPCHECK_EXCLUDES)
    if(EXISTS "${CMAKE_SOURCE_DIR}/cppcheck-excludes.txt")
        file(STRINGS "${CMAKE_SOURCE_DIR}/cppcheck-excludes.txt" CPPCHECK_EXCLUDE_PATHS)
        foreach(CPPCHECK_EXCLUDE_PATH IN LISTS CPPCHECK_EXCLUDE_PATHS)
            # Absolute paths, because Cppcheck matches them against the absolute source paths below
            if(NOT CPPCHECK_EXCLUDE_PATH STREQUAL "" AND NOT CPPCHECK_EXCLUDE_PATH MATCHES "^#")
                list(APPEND CPPCHECK_EXCLUDES -i "${CMAKE_SOURCE_DIR}/${CPPCHECK_EXCLUDE_PATH}")
            endif()
        endforeach()
    endif()

    add_custom_target(cppcheck-analyze
        COMMAND ${CPPCHECK_EXECUTABLE}
                --enable=all
                --check-level=exhaustive
                --inconclusive
                --verbose
                --quiet
                --std=c11
                --force
                --template=gcc
                --suppress=missingIncludeSystem
                --suppress=unusedFunction   # public library API is called by integrators, not by the library
                --suppress=checkersReport
                -I ${CMAKE_SOURCE_DIR}
                -I ${CMAKE_SOURCE_DIR}/inc
                -I ${X430_MISRA_INCLUDE_DIR}
                ${CPPCHECK_EXCLUDES}
                --output-file=${CMAKE_BINARY_DIR}/cppcheck.txt
                ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E cat ${CMAKE_BINARY_DIR}/cppcheck.txt
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Cppcheck static analysis..."
        VERBATIM
    )

    # Cppcheck MISRA C:2012 addon target
    # Runs the addon directly on Cppcheck dump files (see cmake/misra_check.cmake),
    # because not every Cppcheck package can locate and launch its addons. Rule
    # texts are licensed by MISRA and are not shipped with Cppcheck; pass them with
    # -DX430_MISRA_RULE_TEXTS=<file> to get readable messages instead of rule numbers.

    set(X430_MISRA_RULE_TEXTS "" CACHE FILEPATH "Optional MISRA C:2012 rule texts file for Cppcheck")
    option(X430_MISRA_FAIL_ON_VIOLATION "Make 'misra-analyze' fail when the report has violations" ON)

    get_filename_component(CPPCHECK_BIN_DIR ${CPPCHECK_EXECUTABLE} DIRECTORY)
    find_file(X430_MISRA_SCRIPT misra.py
        PATHS ${CPPCHECK_BIN_DIR}/addons
              ${CPPCHECK_BIN_DIR}/../share/cppcheck/addons
              /usr/share/cppcheck/addons
              /usr/local/share/cppcheck/addons
              /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cppcheck/addons   # Debian/Ubuntu packages
        NO_DEFAULT_PATH)
    if(X430_MISRA_SCRIPT AND Python3_Interpreter_FOUND)
        add_custom_target(misra-analyze
            COMMAND ${CMAKE_COMMAND}
                    -DCPPCHECK=${CPPCHECK_EXECUTABLE}
                    -DPYTHON=${Python3_EXECUTABLE}
                    -DMISRA_SCRIPT=${X430_MISRA_SCRIPT}
                    -DRULE_TEXTS=${X430_MISRA_RULE_TEXTS}
                    -DFAIL_ON_VIOLATION=${X430_MISRA_FAIL_ON_VIOLATION}
                    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
                    -DCONFIG_DIR=${X430_MISRA_INCLUDE_DIR}
                    -DOUTPUT=${CMAKE_BINARY_DIR}/misra.txt
                    -P ${CMAKE_SOURCE_DIR}/cmake/misra_check.cmake
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running Cppcheck MISRA C:2012 analysis..."
            VERBATIM
        )
    else()
        message(STATUS "Cppcheck misra.py addon or Python 3 not found. Target 'misra-analyze' will be unavailable.")
    endif()
else()
    message(STATUS "Cppcheck not found. Static analysis targets 'cppcheck-analyze' and 'misra-analyze' will be unavailable.")
endif()

# CodeQL analysis target
# Creates a CodeQL database in the build directory, runs the cpp-queries suite
# and writes a SARIF report (codeql-report.sarif). The target exists only if the
# CodeQL CLI is found on the system.

find_program(CODEQL_EXECUTABLE codeql)
if(CODEQL_EXECUTABLE)

    # Note: This target intentionally builds only the core library; the tests are
    # excluded to keep the analysis focused. Extend if needed.

    if(WIN32)
        set(CODEQL_BUILD_SCRIPT "${CMAKE_BINARY_DIR}/codeql_build.cmd")
        file(WRITE "${CODEQL_BUILD_SCRIPT}" "\"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target x430\r\n")
        set(CODEQL_BUILD_COMMAND "${CODEQL_BUILD_SCRIPT}")
    else()
        set(CODEQL_BUILD_COMMAND "${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target x430")
    endif()

    add_custom_target(codeql-analyze
        COMMAND ${CODEQL_EXECUTABLE} database create
            ${CMAKE_BINARY_DIR}/codeql-db
            --language=c
            --overwrite
            --source-root=${CMAKE_SOURCE_DIR}
            --command "${CODEQL_BUILD_COMMAND}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating CodeQL database..."

        COMMAND ${CODEQL_EXECUTABLE} database analyze
                ${CMAKE_BINARY_DIR}/codeql-db
                codeql/cpp-queries
                --format=sarif-latest
                --output=${CMAKE_BINARY_DIR}/codeql-report.sarif
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running CodeQL analysis..."
        VERBATIM
    )
else()
    message(STATUS "CodeQL not found. Static analysis target 'codeql-analyze' will be unavailable.")
endif()
