################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Description: Library version from the vX.Y.Z Git tag (fallback below).
#              Named x430_version.cmake so it never shadows DXL2-C's
#              version.cmake on the module path.
#
# Author:      Anton Vynohradov
# License:     Apache License 2.0
# SPDX-License-Identifier: Apache-2.0
################################################################################

# Used when the source tree has no Git metadata or no vX.Y.Z tag yet; keep it in
# sync with the fallback values in x430.h.
set(X430_DEFAULT_VERSION "0.1.0")

find_package(Git QUIET)

set(X430_GIT_TAG "")
if(GIT_FOUND AND EXISTS "${X430_ROOT}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${X430_ROOT}
        OUTPUT_VARIABLE X430_GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

string(REGEX MATCH "^[Vv]([0-9]+)\\.([0-9]+)\\.([0-9]+)$" X430_VERSION_MATCH "${X430_GIT_TAG}")
if(NOT X430_VERSION_MATCH)
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" X430_VERSION_MATCH "${X430_DEFAULT_VERSION}")
endif()

set(X430_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(X430_VERSION_MINOR "${CMAKE_MATCH_2}")
set(X430_VERSION_PATCH "${CMAKE_MATCH_3}")
set(X430_VERSION "${X430_VERSION_MAJOR}.${X430_VERSION_MINOR}.${X430_VERSION_PATCH}")

list(APPEND X430_COMPILE_DEFINITIONS
    X430_VERSION_MAJOR=${X430_VERSION_MAJOR}
    X430_VERSION_MINOR=${X430_VERSION_MINOR}
    X430_VERSION_PATCH=${X430_VERSION_PATCH}
    X430_VERSION="${X430_VERSION}"
)

message(STATUS "x430: library version ${X430_VERSION}")
