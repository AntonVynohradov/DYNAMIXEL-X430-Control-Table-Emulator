################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Description: GoogleTest suite, model data checks and the 'coverage' target.
#
# Author:      Anton Vynohradov
# License:     Apache License 2.0
# SPDX-License-Identifier: Apache-2.0
################################################################################

# ==============================================================================
# CMAKE CONFIGURATION
# ==============================================================================

enable_language(CXX)

# Installed GoogleTest first; otherwise a pinned release through FetchContent.
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)
include(CTest)
enable_testing()

# ==============================================================================
# TESTING
# ==============================================================================

set(X430_GTEST ${CMAKE_PROJECT_NAME}_gtest)

add_executable(${X430_GTEST}
    ${X430_ROOT}/tests/unit/x430_test_support.cpp
    ${X430_ROOT}/tests/unit/gtest_x430_protocol.cpp
    ${X430_ROOT}/tests/unit/gtest_x430_group.cpp
    ${X430_ROOT}/tests/unit/gtest_x430_lifecycle.cpp
    ${X430_ROOT}/tests/unit/gtest_x430_sim.cpp
)

set_target_properties(${X430_GTEST} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(${X430_GTEST} PRIVATE ${X430_ROOT}/tests/unit)
target_link_libraries(${X430_GTEST} PRIVATE x430 GTest::gtest GTest::gtest_main)

gtest_discover_tests(${X430_GTEST} PROPERTIES LABELS "unit")

# Model data: generator rules and the transcription against the e-Manual tables.
add_test(NAME x430_models
    COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${X430_ROOT}/tests/models -v
)
set_tests_properties(x430_models PROPERTIES LABELS "models"
    ENVIRONMENT "X430_ROOT=${X430_ROOT}")

# ==============================================================================
# COVERAGE
# ==============================================================================

set(X430_COVERAGE_MIN 95 CACHE STRING "Minimum line coverage (%) required by the 'coverage' target")

find_program(GCOVR_EXECUTABLE gcovr)
if(GCOVR_EXECUTABLE)
    set(COVERAGE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Coverage)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L unit
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
        COMMAND ${GCOVR_EXECUTABLE}
            -r ${X430_ROOT}
            --object-directory ${CMAKE_BINARY_DIR}
            --filter ${X430_ROOT}/src
            --html-details=${COVERAGE_OUTPUT_DIR}/coverage.html
            --print-summary
            --delete
            --fail-under-line ${X430_COVERAGE_MIN}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
    )
else()
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "gcovr not found. Install gcovr to generate coverage reports."
    )
endif()
