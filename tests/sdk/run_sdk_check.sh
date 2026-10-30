#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
#
# Runs tests/sdk/sdk_check.cpp (ROBOTIS DynamixelSDK, C++) against
# examples/host_sim on a pseudo-terminal.
#
#   tests/sdk/run_sdk_check.sh <build dir with X430_EXAMPLES=ON> [DynamixelSDK dir]
#
# Without a DynamixelSDK directory the script clones it into the build directory.
set -euo pipefail

BUILD=$(realpath "$1")
HERE=$(cd "$(dirname "$0")" && pwd)
SDK=${2:-$BUILD/DynamixelSDK}
SIM=$BUILD/examples/x430_host_sim
LINK=$BUILD/ttyDXL

if [ ! -d "$SDK/c++/src/dynamixel_sdk" ]; then
    git clone -q --depth 1 https://github.com/ROBOTIS-GIT/DynamixelSDK.git "$SDK"
fi

SRC=$SDK/c++/src/dynamixel_sdk
g++ -std=c++14 -O1 -I"$SDK/c++/include/dynamixel_sdk" "$HERE/sdk_check.cpp" \
    "$SRC"/group_*.cpp "$SRC"/packet_handler.cpp "$SRC"/port_handler.cpp \
    "$SRC"/port_handler_linux.cpp "$SRC"/protocol1_packet_handler.cpp \
    "$SRC"/protocol2_packet_handler.cpp -o "$BUILD/sdk_check"

MODEL=$( ("$SIM" --help 2>&1 || true) | sed -n 's/.*Model Number \([0-9]*\).*/\1/p')
"$SIM" --pty --link "$LINK" --ids 1,2,3 &
SIM_PID=$!
trap 'kill $SIM_PID 2>/dev/null || true' EXIT
sleep 0.5
"$BUILD/sdk_check" "$LINK" "$MODEL"
