################################################################################
# sfs-dynamixel-x430: host examples
# SPDX-License-Identifier: Apache-2.0
################################################################################

if(UNIX)
    add_executable(x430_host_sim host_sim/x430_host_sim.c)
    target_link_libraries(x430_host_sim PRIVATE x430)
    # POSIX pty/termios API; the library itself stays strict C11.
    set_target_properties(x430_host_sim PROPERTIES C_EXTENSIONS ON)
else()
    message(STATUS "x430: examples/host_sim needs a POSIX system (Linux, macOS, WSL)")
endif()
