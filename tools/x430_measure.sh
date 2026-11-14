#!/usr/bin/env bash
################################################################################
# DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
#
# Project:     DYNAMIXEL-X430-Control-Table-Emulator
# Description: Measures the figures quoted in the documentation: RAM of the
#              instances, flash per module, .data/.bss and the worst-case stack
#              of every public function on Cortex-M4 (arm-none-eabi-gcc), and
#              the instance sizes on the 64-bit host.
#
# Usage:       tools/x430_measure.sh <configured-build-dir> [dxl2-source-dir]
#              (any preset; the generated model header of that build is used)
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# SPDX-License-Identifier: Apache-2.0
################################################################################

set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=$(cd "$1" && pwd)
DXL2=${2:-$BUILD/_deps/dxl2-src}
GEN=$BUILD/x430/generated
OUT=$(mktemp -d)
trap 'rm -rf "$OUT"' EXIT

ARM="arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb"
WARN="-std=c11 -Wall -Wextra -Wpedantic -Wconversion -Werror"
INC="-I$ROOT -I$ROOT/inc -I$GEN -I$DXL2 -I$DXL2/inc"
# Default of x430.cmake and Kconfig.x430 (a READ of the whole Control Table)
PACKET="-DDXL2_MAX_PACKET_SIZE=768U"
LIB_SOURCES="$ROOT/src/x430_device.c $ROOT/src/x430_bus.c $ROOT/src/x430_sim.c $GEN/x430_model_table.c"
DXL2_SOURCES="$DXL2/src/dxl2_protocol.c $DXL2/src/dxl2_link.c"

# Instance sizes: sizeof through a probe object, read back with nm -S
sizes() { # <compiler...> -- <defines...>
    local cc=() defs=()
    while [ "$1" != "--" ]; do cc+=("$1"); shift; done
    shift
    defs=("$@")
    cat > "$OUT/probe.c" <<'EOF'
#include "x430.h"
char x430_probe_bus[sizeof(struct x430_bus)];
char x430_probe_device[sizeof(struct x430_device)];
char x430_probe_sim[sizeof(struct x430_sim)];
EOF
    "${cc[@]}" -c $INC "${defs[@]}" "$OUT/probe.c" -o "$OUT/probe.o"
    local nm=nm
    case "${cc[0]}" in arm-*) nm=arm-none-eabi-nm ;; esac
    "$nm" -S "$OUT/probe.o" | while read -r _ size _ name; do
        printf '%s %d B  ' "${name#x430_probe_}" "$((16#$size))"
    done
    echo
}

echo "## Instance sizes (struct x430_bus / x430_device / x430_sim)"
echo
for packet in 256 768; do
    echo "- Cortex-M4, DXL2_MAX_PACKET_SIZE $packet, all features: $(sizes $ARM -- -DDXL2_MAX_PACKET_SIZE=${packet}U)"
    echo "- x86-64 host, DXL2_MAX_PACKET_SIZE $packet, all features: $(sizes gcc -- -DDXL2_MAX_PACKET_SIZE=${packet}U)"
done
MIN="-DX430_FEATURE_BULK=0 -DX430_FEATURE_FAST_READ=0 -DX430_FEATURE_BACKUP=0 -DX430_FEATURE_SIMULATOR=0"
echo "- Cortex-M4, DXL2_MAX_PACKET_SIZE 768, no optional feature: $(sizes $ARM -- $PACKET $MIN)"
echo "- Cortex-M4, DXL2_MAX_PACKET_SIZE 256, no optional feature: $(sizes $ARM -- -DDXL2_MAX_PACKET_SIZE=256U $MIN)"
echo

flash() { # <opt> <defines...>
    local opt=$1; shift
    rm -f "$OUT"/*.o "$OUT"/*.su "$OUT"/*.ci
    for src in $LIB_SOURCES $DXL2_SOURCES; do
        $ARM $opt $WARN -ffunction-sections -fdata-sections -fstack-usage -fcallgraph-info=su \
            $INC "$@" -c "$src" -o "$OUT/$(basename "${src%.c}").o"
    done
    (cd "$OUT" && arm-none-eabi-size -t x430_device.o x430_bus.o x430_sim.o x430_model_table.o \
        dxl2_protocol.o dxl2_link.o)
}

for opt in -Os -O2; do
    echo "## Flash and data, Cortex-M4 $opt (all features, DXL2_MAX_PACKET_SIZE 768)"
    echo
    echo '```'
    flash $opt $PACKET
    echo '```'
    echo
done

echo "## Flash, Cortex-M4 -Os, no optional feature"
echo
echo '```'
flash -Os $PACKET $MIN
echo '```'
echo

# Worst case of the public functions of x430.h. DXL2-C calls the packet handler of the
# bus through a function pointer: that edge (dxl2_link_receive -> x430_handler) is
# added, so x430_bus_receive() includes the whole instruction handling. The bus frames
# and writes its status packets itself (no DXL2-C send function). The hooks, the port
# functions and the byte writer of the application are not included.
stack_table() {
    PYTHONPATH="$ROOT/tools/docs" python3 - "$OUT" "$ROOT/x430.h" <<'PY'
import re
import sys

import stack_usage as su

stack, calls, _ = su.load(sys.argv[1])
names = {su.short(n): n for n in stack}
calls.setdefault(names["dxl2_link_receive"], set()).add(names["x430_handler"])
public = re.findall(r"^int (x430_\w+)\(", open(sys.argv[2], encoding="utf-8").read(), re.M)
print("| Function | Worst case (bytes) | Deepest chain |")
print("| --- | ---: | --- |")
for name in public:
    depth, chain = su.worst(names[name], stack, calls, [])
    print(f"| `{name}()` | {depth} | {' -> '.join(su.short(c) for c in chain)} |")
PY
}

for opt in -Os -O0; do
    echo "## Worst-case stack, Cortex-M4 $opt (callbacks excluded)"
    echo
    flash $opt $PACKET >/dev/null
    stack_table
    echo
done

echo "## Worst-case stack, Cortex-M4 -Os, DXL2_MAX_PACKET_SIZE 256 (callbacks excluded)"
echo
flash -Os -DDXL2_MAX_PACKET_SIZE=256U >/dev/null
stack_table
