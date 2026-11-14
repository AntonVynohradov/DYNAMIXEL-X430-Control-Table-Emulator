################################################################################
# DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
#
# Project:     DYNAMIXEL-X430-Control-Table-Emulator
# Description: Library target definition. An integrating project includes it
#              directly:   include(path/to/DYNAMIXEL-X430-Control-Table-Emulator/x430.cmake)
#                          target_link_libraries(app PRIVATE x430)
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# Configuration (set before include(), or on the command line):
#   COMPILE_DEFINITIONS     compile definitions of the library (public: the application
#                           sees the same x430_config.h), e.g. X430_MAX_DEVICES=8U
#   COMPILE_OPTIONS         compile options of the library sources only, e.g. -Os
#   UNIT_TESTS              ON: also build the unit tests of this library (x430_gtest),
#                           also when it is a submodule of another project
#   X430_KCONFIG            ON: model and options from Kconfig (Kconfiglib), default OFF
#   X430_CONF_FILES         Kconfig fragments (with X430_KCONFIG)
#   X430_KCONFIG_ROOT       top-level Kconfig (with X430_KCONFIG)
#   X430_MODEL              model key without Kconfig, e.g. xm430_w350
#   X430_MODEL_OVERRIDES    YAML file with application overrides of the model
#   X430_DXL2_GIT_TAG       DXL2-C release fetched with FetchContent (v1.0.0)
#   X430_DXL2_MAX_PACKET_SIZE  DXL2_MAX_PACKET_SIZE without Kconfig (768: a READ of the
#                           whole Control Table, as DYNAMIXEL Wizard does it)
#   FETCHCONTENT_SOURCE_DIR_DXL2  local DXL2-C checkout (offline builds)
#
# COMPILE_DEFINITIONS, COMPILE_OPTIONS and UNIT_TESTS have no prefix on purpose: a
# project sets them once for all the libraries it includes.
#
# The cmake/ folder of this library is deliberately not put on CMAKE_MODULE_PATH:
# DXL2-C includes its own version.cmake by name.
#
# SPDX-License-Identifier: Apache-2.0
################################################################################

# ==============================================================================
# CMAKE CONFIGURATION
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

set(X430_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(FetchContent)

# ==============================================================================
# VERSION EXTRACTION
# ==============================================================================

# Absolute path: an application (and DXL2-C) may have a version.cmake of its own
include(${CMAKE_CURRENT_LIST_DIR}/cmake/version.cmake)

# ==============================================================================
# KCONFIG / MODEL SELECTION
# ==============================================================================

option(X430_KCONFIG "Configure the library with Kconfig (Kconfiglib)" OFF)
set(X430_MODEL "xm430_w350" CACHE STRING "Emulated model without Kconfig (models/x430/<model>.yaml)")
set(X430_DXL2_MAX_PACKET_SIZE 768 CACHE STRING "DXL2_MAX_PACKET_SIZE without Kconfig")
set(X430_MODEL_OVERRIDES "" CACHE FILEPATH
    "Application overrides of the model (YAML, 'overrides:' as in models/); empty = none")
set(COMPILE_DEFINITIONS "" CACHE STRING
    "Compile definitions of the libraries, e.g. X430_MAX_DEVICES=8U;X430_FEATURE_BULK=0")
set(COMPILE_OPTIONS "" CACHE STRING "Compile options of the library sources, e.g. -Os")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(X430_KCONFIG)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/x430_kconfig.cmake)
    set(X430_SELECTED_MODEL "${CONFIG_X430_MODEL_NAME}")
    set(X430_SELECTED_PACKET_SIZE "${CONFIG_X430_DXL2_MAX_PACKET_SIZE}")
    set(X430_SELECTED_OVERRIDES "${CONFIG_X430_MODEL_OVERRIDES}")
    list(APPEND X430_COMPILE_DEFINITIONS X430_HAS_AUTOCONF)
else()
    set(X430_SELECTED_MODEL "${X430_MODEL}")
    set(X430_SELECTED_PACKET_SIZE "${X430_DXL2_MAX_PACKET_SIZE}")
    set(X430_SELECTED_OVERRIDES "${X430_MODEL_OVERRIDES}")
    set(X430_AUTOCONF_DIR "")
endif()

# With or without Kconfig: the definitions given by the project
list(APPEND X430_COMPILE_DEFINITIONS ${COMPILE_DEFINITIONS})

# ==============================================================================
# MODEL TABLE GENERATION
# ==============================================================================

set(X430_GENERATED_DIR ${CMAKE_BINARY_DIR}/x430/generated)

set(X430_OVERRIDES_ARGS "")
if(NOT X430_SELECTED_OVERRIDES STREQUAL "")
    if(NOT EXISTS "${X430_SELECTED_OVERRIDES}")
        message(FATAL_ERROR "x430: model overrides ${X430_SELECTED_OVERRIDES} not found")
    endif()
    set(X430_OVERRIDES_ARGS --overrides ${X430_SELECTED_OVERRIDES})
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${X430_SELECTED_OVERRIDES})
    message(STATUS "x430: model overrides ${X430_SELECTED_OVERRIDES}")
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} ${X430_ROOT}/tools/x430_gen_model.py
            --models-dir ${X430_ROOT}/models
            --model ${X430_SELECTED_MODEL}
            --out-dir ${X430_GENERATED_DIR}
            ${X430_OVERRIDES_ARGS}
    RESULT_VARIABLE X430_GENERATE_RESULT
)
if(X430_GENERATE_RESULT)
    message(FATAL_ERROR "x430: cannot generate the Control Table of model '${X430_SELECTED_MODEL}'")
endif()

# Regenerate when the model data or the generator changes.
file(GLOB_RECURSE X430_MODEL_FILES ${X430_ROOT}/models/*.yaml)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${X430_MODEL_FILES} ${X430_ROOT}/tools/x430_gen_model.py)

message(STATUS "x430: emulated model ${X430_SELECTED_MODEL}")

# ==============================================================================
# DXL2-C (DYNAMIXEL Protocol 2.0 codec and link)
# ==============================================================================

set(X430_DXL2_GIT_TAG "v1.0.0" CACHE STRING "DXL2-C Git tag fetched with FetchContent")

if(NOT TARGET dxl2)
    # SOURCE_SUBDIR points at a directory without CMakeLists.txt: FetchContent only
    # downloads; the library target comes from dxl2.cmake as documented by DXL2-C.
    FetchContent_Declare(dxl2
        GIT_REPOSITORY https://github.com/AntonVynohradov/DXL2-C.git
        GIT_TAG ${X430_DXL2_GIT_TAG}
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR do-not-add-subdirectory
    )
    FetchContent_MakeAvailable(dxl2)

    set(DXL2_UNIT_TESTS OFF)
    list(APPEND DXL2_COMPILE_DEFINITIONS "DXL2_MAX_PACKET_SIZE=${X430_SELECTED_PACKET_SIZE}U")
    include(${dxl2_SOURCE_DIR}/dxl2.cmake)
endif()

# ==============================================================================
# INCLUDE DIRECTORIES
# ==============================================================================

list(APPEND X430_HEADERS

    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${X430_GENERATED_DIR}
)
if(X430_AUTOCONF_DIR)
    list(APPEND X430_HEADERS ${X430_AUTOCONF_DIR})
endif()

# ==============================================================================
# SOURCE FILES
# ==============================================================================

list(APPEND X430_SOURCES

    ${CMAKE_CURRENT_LIST_DIR}/src/x430_device.c
    ${CMAKE_CURRENT_LIST_DIR}/src/x430_bus.c
    ${CMAKE_CURRENT_LIST_DIR}/src/x430_sim.c
    ${X430_GENERATED_DIR}/x430_model_table.c
)

set_source_files_properties(${X430_SOURCES} PROPERTIES LANGUAGE C)

# Library sources only (X430_SOURCES also receives the test sources below)
set(X430_LIBRARY_SOURCES ${X430_SOURCES})

# ==============================================================================
# PROJECT CONFIGURATION
# ==============================================================================

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND X430_COMPILE_DEFINITIONS "X430_DEBUG")
endif()

# ==============================================================================
# LIBRARY TARGET
# ==============================================================================

# Always a static library: with UNIT_TESTS the tests get their own executable, so a
# project that includes this library as a submodule never links the tests.
add_library(x430 STATIC ${X430_LIBRARY_SOURCES})
target_include_directories(x430 PUBLIC ${X430_HEADERS})
target_compile_definitions(x430 PUBLIC ${X430_COMPILE_DEFINITIONS})
target_link_libraries(x430 PUBLIC dxl2)
set_target_properties(x430 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
if(COMPILE_OPTIONS)
    target_compile_options(x430 PRIVATE ${COMPILE_OPTIONS})
endif()

# ==============================================================================
# UNIT TESTS
# ==============================================================================

if(UNIT_TESTS)

    list(APPEND X430_UNIT_TEST_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/x430_test_support.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_x430_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_x430_protocol.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_x430_group.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_x430_lifecycle.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_x430_rules.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_x430_sim.cpp
        # C compile check: x430.h alone, no private header (#error otherwise)
        ${CMAKE_CURRENT_LIST_DIR}/tests/compile/x430_api_check.c
    )

    list(APPEND X430_UNIT_TEST_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit
        ${CMAKE_CURRENT_LIST_DIR}/src        # white-box tests may include src/*.h
    )

    # The test executable compiles the library sources itself (coverage counters); the
    # static library it links is then not used for these symbols.
    set(X430_SOURCES ${X430_LIBRARY_SOURCES} ${X430_UNIT_TEST_SOURCES})

    if(CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
        # This repository is the top-level project: CMakeLists.txt includes
        # cmake/unit_tests.cmake (test executable X430_gtest, coverage gate).
        target_include_directories(x430 PUBLIC ${X430_UNIT_TEST_HEADERS})
    else()
        # A submodule of another project with UNIT_TESTS=ON: the tests of this library
        # are registered with the CTest of that project (label x430).
        enable_language(CXX)
        find_package(GTest REQUIRED)
        include(GoogleTest)
        add_executable(x430_gtest ${X430_SOURCES})
        set_target_properties(x430_gtest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
        target_include_directories(x430_gtest PRIVATE ${X430_UNIT_TEST_HEADERS})
        target_link_libraries(x430_gtest PRIVATE x430 GTest::gtest GTest::gtest_main)
        gtest_discover_tests(x430_gtest PROPERTIES LABELS "x430" DISCOVERY_TIMEOUT 60)
    endif()

endif()
