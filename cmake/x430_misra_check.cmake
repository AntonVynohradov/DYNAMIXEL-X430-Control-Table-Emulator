################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Description: Cppcheck MISRA C:2012 addon run on dump files (same mechanism as
#              DXL2-C: not every Cppcheck package can launch its addons).
#
# Inputs (-D): CPPCHECK, PYTHON, MISRA_SCRIPT, SOURCE_DIR, OUTPUT, INCLUDE_DIRS,
#              RULE_TEXTS (optional), FAIL_ON_VIOLATION (optional)
#
# SPDX-License-Identifier: Apache-2.0
################################################################################

file(GLOB MISRA_SOURCES ${SOURCE_DIR}/src/*.c)

set(MISRA_INCLUDES)
foreach(MISRA_INCLUDE IN LISTS INCLUDE_DIRS)
    if(MISRA_INCLUDE)
        list(APPEND MISRA_INCLUDES -I ${MISRA_INCLUDE})
    endif()
endforeach()

execute_process(
    COMMAND ${CPPCHECK} --dump --quiet --std=c11 --suppress=missingIncludeSystem
            -DX430_HAS_AUTOCONF ${MISRA_INCLUDES} ${MISRA_SOURCES}
    RESULT_VARIABLE MISRA_DUMP_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
)
if(NOT MISRA_DUMP_RESULT EQUAL 0)
    message(FATAL_ERROR "Cppcheck --dump failed with code ${MISRA_DUMP_RESULT}")
endif()

set(MISRA_DUMPS)
foreach(MISRA_SOURCE IN LISTS MISRA_SOURCES)
    list(APPEND MISRA_DUMPS ${MISRA_SOURCE}.dump)
endforeach()

set(MISRA_ARGS)
if(RULE_TEXTS)
    list(APPEND MISRA_ARGS --rule-texts=${RULE_TEXTS})
endif()

execute_process(
    COMMAND ${PYTHON} ${MISRA_SCRIPT} ${MISRA_ARGS} ${MISRA_DUMPS}
    RESULT_VARIABLE MISRA_RESULT
    OUTPUT_VARIABLE MISRA_STDOUT
    ERROR_VARIABLE MISRA_REPORT
)

foreach(MISRA_SOURCE IN LISTS MISRA_SOURCES)
    file(REMOVE ${MISRA_SOURCE}.dump ${MISRA_SOURCE}.ctu-info)
endforeach()

file(WRITE ${OUTPUT} "${MISRA_REPORT}${MISRA_STDOUT}")
message("${MISRA_REPORT}${MISRA_STDOUT}")

string(REGEX MATCHALL "\\[misra-c2012-[0-9.]+\\]" MISRA_VIOLATIONS "${MISRA_REPORT}")
list(LENGTH MISRA_VIOLATIONS MISRA_VIOLATION_COUNT)
message(STATUS "MISRA C:2012 violations: ${MISRA_VIOLATION_COUNT} (report: ${OUTPUT})")

if(FAIL_ON_VIOLATION AND MISRA_VIOLATION_COUNT GREATER 0)
    message(FATAL_ERROR "MISRA C:2012 violations found; fix them or record a deviation in docs/misra.md")
endif()
