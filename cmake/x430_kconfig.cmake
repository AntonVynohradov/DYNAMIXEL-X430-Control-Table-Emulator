################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Project:     sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
# Description: Kconfig integration (Kconfiglib): .config, x430_autoconf.h,
#              CONFIG_* CMake variables and the menuconfig/guiconfig/
#              savedefconfig targets.
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# Inputs:
#   X430_KCONFIG_ROOT  top-level Kconfig (default: <library>/Kconfig). A project
#                      with its own Kconfig tree points it at its file, which
#                      sources Kconfig.x430.
#   X430_CONF_FILES    fragments applied in order (default: configs/defconfig)
#
# Outputs:
#   X430_AUTOCONF_DIR  directory of x430_autoconf.h
#   CONFIG_X430_*      CMake variables with the configured values
#
# Flow: the configure step merges the fragments into <build>/x430/.config
# (kept as long as the fragments do not change, so menuconfig edits survive).
# .config is a configure dependency: after menuconfig the next build re-runs
# CMake, which regenerates the header and the model table.
#
# SPDX-License-Identifier: Apache-2.0
################################################################################

find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import kconfiglib"
    RESULT_VARIABLE X430_KCONFIGLIB_MISSING
    OUTPUT_QUIET ERROR_QUIET
)
if(X430_KCONFIGLIB_MISSING)
    message(FATAL_ERROR
        "Kconfiglib not found for ${Python3_EXECUTABLE}.\n"
        "Install it with: ${Python3_EXECUTABLE} -m pip install -r ${X430_ROOT}/requirements.txt\n"
        "or configure without Kconfig: -DX430_KCONFIG=OFF -DX430_MODEL=<model>")
endif()

set(X430_KCONFIG_ROOT "${X430_ROOT}/Kconfig" CACHE FILEPATH "Top-level Kconfig file")
set(X430_CONF_FILES "${X430_ROOT}/configs/defconfig" CACHE STRING
    "Kconfig fragments applied in order (;-separated)")

set(X430_KCONFIG_DIR ${CMAKE_BINARY_DIR}/x430)
set(X430_DOTCONFIG ${X430_KCONFIG_DIR}/.config)
set(X430_AUTOCONF_DIR ${X430_KCONFIG_DIR}/include)
set(X430_KCONFIG_CMAKE ${X430_KCONFIG_DIR}/x430_config.cmake)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env srctree=${X430_ROOT}
            ${Python3_EXECUTABLE} ${X430_ROOT}/tools/x430_kconfig.py
            --kconfig ${X430_KCONFIG_ROOT} --config ${X430_DOTCONFIG}
            configure
            --header ${X430_AUTOCONF_DIR}/x430_autoconf.h
            --cmake ${X430_KCONFIG_CMAKE}
            ${X430_CONF_FILES}
    RESULT_VARIABLE X430_KCONFIG_RESULT
)
if(X430_KCONFIG_RESULT)
    message(FATAL_ERROR "x430: Kconfig processing failed")
endif()

include(${X430_KCONFIG_CMAKE})

# Re-run CMake when the configuration or the Kconfig tree changes.
file(GLOB X430_KCONFIG_FILES ${X430_ROOT}/Kconfig*)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${X430_DOTCONFIG} ${X430_CONF_FILES} ${X430_KCONFIG_ROOT} ${X430_KCONFIG_FILES})

# Interactive configuration (edits <build>/x430/.config).
set(X430_KCONFIG_ENV ${CMAKE_COMMAND} -E env
    srctree=${X430_ROOT} KCONFIG_CONFIG=${X430_DOTCONFIG})

if(NOT TARGET menuconfig)
    add_custom_target(menuconfig
        COMMAND ${X430_KCONFIG_ENV} ${Python3_EXECUTABLE} -m menuconfig ${X430_KCONFIG_ROOT}
        WORKING_DIRECTORY ${X430_KCONFIG_DIR}
        COMMENT "Kconfig: menuconfig (terminal UI)"
        USES_TERMINAL
    )
    add_custom_target(guiconfig
        COMMAND ${X430_KCONFIG_ENV} ${Python3_EXECUTABLE} -m guiconfig ${X430_KCONFIG_ROOT}
        WORKING_DIRECTORY ${X430_KCONFIG_DIR}
        COMMENT "Kconfig: guiconfig (Tk window)"
        USES_TERMINAL
    )
    add_custom_target(savedefconfig
        COMMAND ${CMAKE_COMMAND} -E env srctree=${X430_ROOT}
                ${Python3_EXECUTABLE} ${X430_ROOT}/tools/x430_kconfig.py
                --kconfig ${X430_KCONFIG_ROOT} --config ${X430_DOTCONFIG}
                savedefconfig --output ${CMAKE_BINARY_DIR}/defconfig
        COMMENT "Kconfig: minimal configuration -> ${CMAKE_BINARY_DIR}/defconfig"
    )
endif()

message(STATUS "x430: Kconfig ${X430_KCONFIG_ROOT}, fragments: ${X430_CONF_FILES}")
