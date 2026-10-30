################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Project:     sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
# Description: Top-level CMake file: compiler settings, library, tests,
#              examples and static analysis targets.
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# References:
#   - DYNAMIXEL Protocol 2.0: https://docs.robotis.com/docs/dxl/protocol/protocol2/
#   - DYNAMIXEL X series:     https://docs.robotis.com/docs/dxl/model_reference/x_series/
#   - DXL2-C:                 https://github.com/AntonVynohradov/DXL2-C
#
# Last Updated: October 2026
# SPDX-License-Identifier: Apache-2.0
#
################################################################################

# ==============================================================================
# CMAKE CONFIGURATION
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

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

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -Wconversion -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -Wconversion -O2 -DNDEBUG")

# ==============================================================================
# LIBRARY/EXECUTABLE TARGETS
# ==============================================================================

option(X430_UNIT_TESTS "Enable X430 unit tests (GoogleTest)" OFF)
option(X430_EXAMPLES "Build the host examples (serial/pty simulator)" OFF)

if(X430_UNIT_TESTS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # Coverage counters in the library objects, read by the 'coverage' target
    set(X430_COMPILE_OPTIONS -O0 -g --coverage)
endif()

# The static library target (and DXL2-C) is defined in x430.cmake
include(x430)

if(X430_UNIT_TESTS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(x430 PUBLIC --coverage)
endif()

# ==============================================================================
# UNIT TESTS
# ==============================================================================

if(X430_UNIT_TESTS)
    include(x430_unit_tests)
endif()

# ==============================================================================
# EXAMPLES
# ==============================================================================

if(X430_EXAMPLES)
    enable_testing()
    add_subdirectory(examples)
endif()

# ==============================================================================
# STATIC ANALYSIS TARGETS
# ==============================================================================

# Cppcheck: all checks, report in <build>/cppcheck.txt
find_program(CPPCHECK_EXECUTABLE cppcheck)
if(CPPCHECK_EXECUTABLE)
    add_custom_target(cppcheck-analyze
        COMMAND ${CPPCHECK_EXECUTABLE}
                --enable=all
                --check-level=exhaustive
                --inconclusive
                --quiet
                --std=c11
                --force
                --template=gcc
                --suppress=missingIncludeSystem
                --suppress=unusedFunction
                -I ${CMAKE_SOURCE_DIR}
                -I ${CMAKE_SOURCE_DIR}/inc
                -I ${X430_GENERATED_DIR}
                -I ${dxl2_SOURCE_DIR}/inc
                -I ${dxl2_SOURCE_DIR}
                --output-file=${CMAKE_BINARY_DIR}/cppcheck.txt
                ${CMAKE_SOURCE_DIR}/src
        COMMAND ${CMAKE_COMMAND} -E cat ${CMAKE_BINARY_DIR}/cppcheck.txt
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Cppcheck static analysis..."
        VERBATIM
    )

    # MISRA C:2012 addon, same mechanism as DXL2-C (cmake/x430_misra_check.cmake)
    set(X430_MISRA_RULE_TEXTS "" CACHE FILEPATH "Optional MISRA C:2012 rule texts file for Cppcheck")
    option(X430_MISRA_FAIL_ON_VIOLATION "Make 'misra-analyze' fail when the report has violations" ON)

    get_filename_component(CPPCHECK_BIN_DIR ${CPPCHECK_EXECUTABLE} DIRECTORY)
    find_file(X430_MISRA_SCRIPT misra.py
        PATHS ${CPPCHECK_BIN_DIR}/addons
              ${CPPCHECK_BIN_DIR}/../share/cppcheck/addons
              /usr/share/cppcheck/addons
              /usr/local/share/cppcheck/addons
              /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cppcheck/addons
        NO_DEFAULT_PATH)

    if(X430_MISRA_SCRIPT)
        add_custom_target(misra-analyze
            COMMAND ${CMAKE_COMMAND}
                    -DCPPCHECK=${CPPCHECK_EXECUTABLE}
                    -DPYTHON=${Python3_EXECUTABLE}
                    -DMISRA_SCRIPT=${X430_MISRA_SCRIPT}
                    -DRULE_TEXTS=${X430_MISRA_RULE_TEXTS}
                    -DFAIL_ON_VIOLATION=${X430_MISRA_FAIL_ON_VIOLATION}
                    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
                    "-DINCLUDE_DIRS=${CMAKE_SOURCE_DIR}/inc;${CMAKE_SOURCE_DIR};${X430_GENERATED_DIR};${X430_AUTOCONF_DIR};${dxl2_SOURCE_DIR}/inc;${dxl2_SOURCE_DIR}"
                    -DOUTPUT=${CMAKE_BINARY_DIR}/misra.txt
                    -P ${CMAKE_SOURCE_DIR}/cmake/x430_misra_check.cmake
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running Cppcheck MISRA C:2012 analysis..."
            VERBATIM
        )
    else()
        message(STATUS "Cppcheck misra.py addon not found. Target 'misra-analyze' will be unavailable.")
    endif()
else()
    message(STATUS "Cppcheck not found. Static analysis targets 'cppcheck-analyze' and 'misra-analyze' will be unavailable.")
endif()
