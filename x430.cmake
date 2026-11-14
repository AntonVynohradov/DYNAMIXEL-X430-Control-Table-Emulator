################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Project:     sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
# Description: Library target definition. An integrating project includes it
#              directly:   include(path/to/sfs-dynamixel-x430/x430.cmake)
#                          target_link_libraries(app PRIVATE x430)
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# Configuration (set before include, or on the command line):
#   X430_KCONFIG            ON: model and options from Kconfig (default ON)
#   X430_CONF_FILES         Kconfig fragments (with X430_KCONFIG)
#   X430_KCONFIG_ROOT       top-level Kconfig (with X430_KCONFIG)
#   X430_MODEL              model key without Kconfig, e.g. xm430_w350
#   X430_DXL2_GIT_TAG       DXL2-C release fetched with FetchContent (v1.0.0)
#   X430_DXL2_MAX_PACKET_SIZE  DXL2_MAX_PACKET_SIZE without Kconfig (256)
#   FETCHCONTENT_SOURCE_DIR_DXL2  local DXL2-C checkout (offline builds)
#
# Without Kconfig, the options of x430_config.h are plain compiler
# definitions: target_compile_definitions(x430 PUBLIC X430_MAX_DEVICES=8U).
#
# SPDX-License-Identifier: Apache-2.0
################################################################################

# ==============================================================================
# CMAKE CONFIGURATION
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

set(X430_ROOT ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH ${X430_ROOT}/cmake)

include(FetchContent)
include(x430_version)

option(X430_KCONFIG "Configure the library with Kconfig (Kconfiglib)" ON)

# ==============================================================================
# KCONFIG / MODEL SELECTION
# ==============================================================================

set(X430_MODEL "xm430_w350" CACHE STRING "Emulated model without Kconfig (models/x430/<model>.yaml)")
set(X430_DXL2_MAX_PACKET_SIZE 256 CACHE STRING "DXL2_MAX_PACKET_SIZE without Kconfig")
set(X430_MODEL_OVERRIDES "" CACHE FILEPATH
    "Application overrides of the model (YAML, 'overrides:' as in models/); empty = none")

if(X430_KCONFIG)
    include(x430_kconfig)
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

# ==============================================================================
# MODEL TABLE GENERATION
# ==============================================================================

find_package(Python3 REQUIRED COMPONENTS Interpreter)

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
# LIBRARY TARGET
# ==============================================================================

list(APPEND X430_SOURCES
    ${X430_ROOT}/src/x430_device.c
    ${X430_ROOT}/src/x430_bus.c
    ${X430_ROOT}/src/x430_sim.c
    ${X430_GENERATED_DIR}/x430_model_table.c
)

list(APPEND X430_HEADERS
    ${X430_ROOT}
    ${X430_ROOT}/inc
    ${X430_GENERATED_DIR}
)
if(X430_AUTOCONF_DIR)
    list(APPEND X430_HEADERS ${X430_AUTOCONF_DIR})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND X430_COMPILE_DEFINITIONS "X430_DEBUG")
endif()

add_library(x430 STATIC ${X430_SOURCES})
target_include_directories(x430 PUBLIC ${X430_HEADERS} PRIVATE ${X430_ROOT}/src)
target_compile_definitions(x430 PUBLIC ${X430_COMPILE_DEFINITIONS})
target_link_libraries(x430 PUBLIC dxl2)
set_target_properties(x430 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)

if(X430_COMPILE_OPTIONS)
    target_compile_options(x430 PRIVATE ${X430_COMPILE_OPTIONS})
endif()
