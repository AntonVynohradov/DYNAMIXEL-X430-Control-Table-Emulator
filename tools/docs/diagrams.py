#******************************************************************************
# DYNAMIXEL-X430-Control-Table-Emulator: DYNAMIXEL X430 Control Table Emulator
#
# Description: Generates the SVG diagrams of the documentation
#              (docs/assets/images/*.svg). Run after changing a diagram:
#                  python tools/docs/diagrams.py
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright (c) 2026 Anton Vynohradov
#
# SPDX-License-Identifier: Apache-2.0
#*****************************************************************************

"""Documentation diagrams of DYNAMIXEL-X430-Control-Table-Emulator, built with svgkit.

The structure (contexts, modules, states, the write pipeline, the Fast Sync Read packet)
is taken from the code; the motion profile is computed with the integer arithmetic of
src/x430_sim.c, so the curve is the one the simulator produces."""

from __future__ import annotations

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from svgkit import (BLUE, GREEN, GREY, HERO_DEFS, HERO_STYLE, LIGHT, LINE, NAVY,  # noqa: E402
                    ORANGE, RED, TEAL, WHITE, Iso, Svg)

OUT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..",
                                    "docs", "assets", "images"))

SOFT_TEAL = "#E6F4F5"
SOFT_ORANGE = "#FFF7EC"
SOFT_RED = "#FDECEC"
SOFT_GREEN = "#EAF5EA"
SOFT_BLUE = "#EAF1FB"

# ==============================================================================
# PROJECT DATA
# ==============================================================================

CONTEXTS = [
    ("RX thread or main loop", ["x430_bus_receive(bytes)", "x430_bus_poll() every 5-20 ms"]),
    ("Motor control loop", ["x430_bus_lock() · x430_device_set(Present ...)",
                            "x430_bus_unlock() · x430_sim_step()"]),
]

MODULES = [
    ("Bus — src/x430_bus.c", ["DXL2-C link · instruction decoding · Status Return Level · "
                              "SYNC / BULK / FAST group reads · Bus Watchdog"]),
    ("Device — src/x430_device.c", ["Control Table image", "rules · REG_WRITE · persistence"]),
    ("Model — generated", ["x430_model_table.c", "from models/*.yaml"]),
    ("Simulator — src/x430_sim.c", ["kinematic model", "profiles · live registers"]),
]

CALLBACKS = "write(packet)  ·  port: lock / millis / delay_us  ·  hooks: write / read / event / nvm"
BOARD = ("Application and board", ["UART · motor driver · flash"])

HERO_CHIP = ("X430 EMULATOR", "CONTROL TABLE · PROTOCOL 2.0")


# ==============================================================================
# HELPERS
# ==============================================================================

def box(s: Svg, x, y, w, h, title, lines=(), fill=LIGHT, stroke=LINE, color=NAVY,
        size=15, mono_lines=False) -> None:
    """A rounded box with a bold title and smaller lines below it."""
    s.rect(x, y, w, h, fill=fill, stroke=stroke, rx=10)
    top = y + h / 2 - (len(lines) * 17) / 2 + (5 if not lines else -2)
    s.text(x + w / 2, top, title, size=size, weight="bold", fill=color)
    for i, value in enumerate(lines):
        s.text(x + w / 2, top + 20 + i * 17, value, size=12.5, fill=GREY, mono=mono_lines)


def arrow(s: Svg, x1, y1, x2, y2, color=GREY, label="", dy=-8, size=12, dash=False,
          anchor="middle", dx=0) -> None:
    kind = {TEAL: "teal", ORANGE: "orange", GREEN: "green"}.get(color, "end")
    s.line(x1, y1, x2, y2, color=color, arrow=kind, dash=dash)
    if label:
        s.text((x1 + x2) / 2 + dx, (y1 + y2) / 2 + dy, label, size=size, fill=color,
               anchor=anchor)


# ==============================================================================
# MODULES
# ==============================================================================

def architecture() -> None:
    s = Svg(960, 600, "DYNAMIXEL-X430-Control-Table-Emulator modules and the application "
                      "around them")
    width = 400
    for i, (title, lines) in enumerate(CONTEXTS):
        x = 40 + i * (width + 80)
        box(s, x, 30, width, 90, title, lines, mono_lines=True)
        arrow(s, x + width / 2, 120, x + width / 2, 172)
    s.rect(20, 172, 920, 300, fill=WHITE, stroke=BLUE, rx=14, sw=2)
    s.text(40, 200, "X430 emulator", size=17, weight="bold", fill=BLUE, anchor="start")
    s.text(190, 200, "public API: x430.h · inc/x430_types.h · inc/x430_defs.h · "
           "inc/x430_config.h · inc/x430_model.h", size=12.5, fill=GREY, anchor="start")
    title, lines = MODULES[0]
    box(s, 40, 216, 880, 76, title, lines, fill=SOFT_TEAL, stroke=TEAL)
    rest = MODULES[1:]
    w = (880 - 20 * (len(rest) - 1)) / len(rest)
    for i, (title, lines) in enumerate(rest):
        box(s, 40 + i * (w + 20), 310, w, 92, title, lines)
    s.rect(40, 418, 880, 40, fill=SOFT_BLUE, stroke=BLUE, rx=8, dash=True)
    s.text(480, 443, "DXL2-C 1.0.0 (dependency): Protocol 2.0 parser, encoder, CRC, "
           "byte stuffing", size=12.5, fill=BLUE)
    s.text(480, 486, "private: everything under src/; struct x430_bus, x430_device, x430_sim "
           "are opaque", size=12, fill=GREY)
    arrow(s, 480, 494, 480, 520, color=ORANGE)
    s.text(492, 512, CALLBACKS, size=12, fill=ORANGE, anchor="start")
    box(s, 260, 522, 440, 66, BOARD[0], BOARD[1], fill=SOFT_ORANGE, stroke=ORANGE)
    s.save(OUT + "/architecture.svg")


# ==============================================================================
# DEVICE STATES
# ==============================================================================

def state_machine() -> None:
    s = Svg(940, 420, "States of an emulated device")
    w, h = 210, 62
    states = {
        "NOT INITIALIZED": (30, 40, LIGHT, GREY),
        "TORQUE OFF": (360, 40, SOFT_BLUE, BLUE),
        "TORQUE ON": (690, 40, SOFT_GREEN, GREEN),
        "SHUTDOWN": (360, 300, SOFT_RED, RED),
        "WATCHDOG ERROR": (690, 300, SOFT_ORANGE, ORANGE),
    }
    for name, (x, y, fill, color) in states.items():
        s.rect(x, y, w, h, fill=fill, stroke=color, rx=31, sw=2)
        s.text(x + w / 2, y + 38, name, size=14, weight="bold", fill=color, mono=True)

    def mid(name, side):
        x, y = states[name][0], states[name][1]
        return {"r": (x + w, y + h / 2), "l": (x, y + h / 2), "b": (x + w / 2, y + h),
                "t": (x + w / 2, y)}[side]

    (x1, y1), (x2, y2) = mid("NOT INITIALIZED", "r"), mid("TORQUE OFF", "l")
    arrow(s, x1, y1, x2, y2, color=GREEN, label="x430_device_init()", dy=-10)
    (x1, y1), (x2, y2) = mid("TORQUE OFF", "r"), mid("TORQUE ON", "l")
    arrow(s, x1, y1 - 12, x2, y2 - 12, color=GREEN, label="Torque Enable = 1", dy=-10)
    s.text((x1 + x2) / 2, 22, "(also Startup Configuration bit 0, Drive Mode bit 3)",
           size=11.5, fill=GREY)
    arrow(s, x2, y2 + 12, x1, y1 + 12, color=GREY, label="Torque Enable = 0", dy=22)
    s.path("M720,102 L720,230 L465,230 L465,300", color=RED, arrow="end")
    s.text(710, 200, "hardware error in Shutdown (63):", size=12, fill=RED, anchor="end")
    s.text(710, 216, "x430_device_raise_hardware_error()", size=11.5, fill=RED,
           anchor="end", mono=True)
    s.line(825, 102, 825, 300, color=ORANGE, arrow="orange")
    s.text(815, 262, "no instruction packet for", size=12, fill=ORANGE, anchor="end")
    s.text(815, 278, "Bus Watchdog x 20 ms", size=12, fill=ORANGE, anchor="end")
    x, y = mid("SHUTDOWN", "l")
    s.path(f"M{x},{y} L{135},{y} L{135},{102}", color=GREY, arrow="end")
    s.text(145, 300, "REBOOT: RAM reset,", size=12, fill=GREY, anchor="start")
    s.text(145, 316, "errors cleared", size=12, fill=GREY, anchor="start")
    s.text(700, 384, "goal registers answer Data Range Error until Bus Watchdog is written",
           size=11.5, fill=ORANGE)
    s.text(470, 412, "Every function but the *_init() functions returns X430_ERR_STATE "
           "before a successful initialization.", size=12, fill=GREY)
    s.save(OUT + "/state-machine.svg")


# ==============================================================================
# INTEGRATION TIMING
# ==============================================================================

def integration() -> None:
    s = Svg(980, 330, "The emulator on a microcontroller: UART interrupt, RX thread, motor loop")
    x0, x1 = 170, 940
    rows = [("UART RX interrupt", 60, TEAL), ("RX thread", 140, BLUE),
            ("UART TX", 210, ORANGE), ("Motor control loop", 280, GREEN)]
    for name, y, color in rows:
        s.text(x0 - 14, y + 5, name, size=12.5, fill=color, anchor="end")
        s.line(x0, y + 18, x1, y + 18, color=LINE)
    for k, x in enumerate((190, 215, 240, 265)):
        s.rect(x, 50, 18, 22, fill=SOFT_TEAL, stroke=TEAL, rx=3)
    s.text(240, 40, "bytes -> ring buffer", size=11.5, fill=TEAL)
    s.rect(300, 126, 300, 30, fill=SOFT_BLUE, stroke=BLUE, rx=5)
    s.text(450, 146, "x430_bus_receive(): parse, check, hooks", size=12, fill=BLUE, mono=True)
    arrow(s, 290, 72, 305, 124, color=TEAL)
    s.rect(610, 196, 120, 28, fill=SOFT_ORANGE, stroke=ORANGE, rx=5)
    s.text(670, 215, "status packet", size=12, fill=ORANGE)
    s.text(612, 182, "after Return Delay Time", size=11, fill=GREY, anchor="start")
    arrow(s, 600, 156, 612, 196, color=ORANGE)
    s.rect(760, 126, 120, 30, fill=SOFT_BLUE, stroke=BLUE, rx=5)
    s.text(820, 146, "x430_bus_poll()", size=12, fill=BLUE, mono=True)
    for x in (190, 450, 710):
        s.rect(x, 268, 200, 28, fill=SOFT_GREEN, stroke=GREEN, rx=5)
        s.text(x + 100, 287, "lock · set Present · unlock", size=11.5, fill=GREEN)
    s.text(555, 320, "The bus lock (x430_port) separates the RX thread and the motor loop; "
           "hooks run inside x430_bus_receive() with the lock held.", size=12, fill=NAVY)
    s.save(OUT + "/integration.svg")


# ==============================================================================
# WRITE PIPELINE
# ==============================================================================

def write_pipeline() -> None:
    s = Svg(980, 300, "How a WRITE is executed")
    steps = [
        ("1  Address", ["length 0: Data Length", "beyond table: Access"], LIGHT, GREY),
        ("2  Indirect", ["Indirect Data resolved", "to the real registers"], LIGHT, GREY),
        ("3  Check all", ["access, EEPROM lock,", "range, limits, watchdog"], SOFT_TEAL, TEAL),
        ("4  Hook + store", ["write hook per register", "in address order"], SOFT_BLUE, BLUE),
        ("5  Persist", ["EEPROM changed:", "nvm_store()"], SOFT_ORANGE, ORANGE),
        ("6  Follow-ups", ["Torque On by Goal;", "Goal := Present"], SOFT_GREEN, GREEN),
    ]
    w, gap = 140, 22
    for i, (title, lines, fill, color) in enumerate(steps):
        x = 20 + i * (w + gap)
        box(s, x, 60, w, 100, title, lines, fill=fill, stroke=color, color=color, size=14)
        if i:
            arrow(s, x - gap, 110, x, 110)
    s.text(490, 30, "x430_device_write() — WRITE, SYNC_WRITE, BULK_WRITE, ACTION and "
           "Indirect Data take the same path", size=13, fill=NAVY)
    s.text(270, 196, "one failed check: nothing is stored,", size=12, fill=TEAL)
    s.text(270, 212, "the error goes into the status packet", size=12, fill=TEAL)
    s.text(600, 196, "a refusing hook stops the write at its register", size=12, fill=BLUE)
    s.text(490, 260, "REG_WRITE runs steps 1-3 only and keeps the data for ACTION.",
           size=12.5, fill=GREY)
    s.save(OUT + "/write-pipeline.svg")


# ==============================================================================
# FAST SYNC READ PACKET
# ==============================================================================

def fast_read() -> None:
    s = Svg(830, 220, "Fast Sync Read status packet with two devices")
    fields = [
        ("Header", 20, 150, ["FF", "FF", "FD", "00"], NAVY),
        ("ID", 170, 40, ["FE"], NAVY),
        ("Length", 210, 80, ["L", "H"], NAVY),
        ("Inst", 290, 40, ["55"], NAVY),
        ("Device 1", 330, 240, ["err", "ID", "data", "CRC1"], TEAL),
        ("Device 2", 570, 240, ["err", "ID", "data", "CRC2"], BLUE),
    ]
    s.field_row(60, fields)
    s.bracket(20, 510, 140, "CRC1: from the header to the data of device 1", color=TEAL)
    s.bracket(20, 750, 190, "CRC2 = packet CRC: from the header to the data of device 2",
              color=BLUE)
    s.save(OUT + "/fast-read.svg")


# ==============================================================================
# MOTION PROFILE (integer model of src/x430_sim.c)
# ==============================================================================

VELOCITY_UNIT = 15633        # X430_SIM_VELOCITY_UNIT
ACCELERATION_UNIT = 244140   # X430_SIM_ACCELERATION_UNIT
MILLI = 1000


def _isqrt(value: int) -> int:
    result, bit = 0, 1 << 62
    while bit > value:
        bit >>= 2
    while bit:
        if value >= result + bit:
            value -= result + bit
            result = (result >> 1) + bit
        else:
            result >>= 1
        bit >>= 2
    return result


def _trunc_div(a: int, b: int) -> int:
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b > 0) else -q


def simulate_move(goal: int, profile_velocity: int, profile_acceleration: int):
    """x430_sim_tick_position() of src/x430_sim.c, one step per millisecond."""
    vmax = profile_velocity * VELOCITY_UNIT
    acceleration = profile_acceleration * ACCELERATION_UNIT
    shaft, velocity, samples = 0, 0, []
    for t in range(20000):
        error = goal * MILLI - shaft
        distance = abs(error)
        reach = distance * MILLI
        if acceleration:
            reach = min(reach, _isqrt(2 * acceleration * min(distance, 10 ** 9)))
        reach = min(reach, vmax)
        desired = reach if error >= 0 else -reach
        step = acceleration // MILLI
        if step and desired > velocity + step:
            velocity += step
        elif step and desired < velocity - step:
            velocity -= step
        else:
            velocity = desired
        move = _trunc_div(velocity, MILLI)
        if distance <= abs(move) or distance < MILLI // 2:
            shaft += error
            velocity = 0
        else:
            shaft += move
        samples.append((t + 1, shaft / MILLI, velocity / VELOCITY_UNIT))
        if shaft == goal * MILLI and velocity == 0:
            break
    return samples


def profile() -> None:
    goal, pv, pa = 4096, 100, 20
    samples = simulate_move(goal, pv, pa)
    end = samples[-1][0]
    s = Svg(960, 420, "Trapezoidal profile of the simulator: Present Velocity and Position")
    x0, x1, yv0, yv1, yp0, yp1 = 90, 900, 190, 50, 380, 240
    tmax = (end // 500 + 1) * 500

    def tx(t):
        return x0 + (x1 - x0) * t / tmax

    for t in range(0, tmax + 1, 500):
        s.line(tx(t), 50, tx(t), 380, color=LINE, dash=True)
        s.text(tx(t), 398, f"{t}", size=11.5, fill=GREY)
    s.text((x0 + x1) / 2, 416, "time [ms]", size=12, fill=GREY)
    s.line(x0, yv0, x1, yv0, color=GREY)
    s.line(x0, yp0, x1, yp0, color=GREY)
    s.text(x0 - 10, yv1 + 4, f"{pv}", size=11.5, fill=TEAL, anchor="end")
    s.text(x0 - 10, yv0 + 4, "0", size=11.5, fill=TEAL, anchor="end")
    s.text(x0 - 10, yp1 + 4, f"{goal}", size=11.5, fill=BLUE, anchor="end")
    s.text(x0 - 10, yp0 + 4, "0", size=11.5, fill=BLUE, anchor="end")
    s.text(30, 120, "Present Velocity", size=12, fill=TEAL, rotate=-90)
    s.text(30, 310, "Present Position", size=12, fill=BLUE, rotate=-90)
    vel = " ".join(f"{'M' if i == 0 else 'L'}{tx(t):.1f},{yv0 + (yv1 - yv0) * v / pv:.1f}"
                   for i, (t, _, v) in enumerate([(0, 0, 0)] + samples))
    pos = " ".join(f"{'M' if i == 0 else 'L'}{tx(t):.1f},{yp0 + (yp1 - yp0) * p / goal:.1f}"
                   for i, (t, p, _) in enumerate([(0, 0, 0)] + samples))
    s.path(vel, color=TEAL, width=2.2)
    s.path(pos, color=BLUE, width=2.2)
    s.text(x1, 40, f"Goal Position {goal} (one turn), Profile Velocity {pv}, "
           f"Profile Acceleration {pa}: {end} ms", size=12.5, fill=NAVY, anchor="end")
    s.save(OUT + "/profile.svg")


# ==============================================================================
# HERO
# ==============================================================================

def hero() -> None:
    """Animated isometric hero: a host adapter sends Protocol 2.0 packets over the
    DYNAMIXEL bus to a controller board; the chip of the library answers as an X430 and
    drives a stepper motor that is not a DYNAMIXEL (phase currents on its cable)."""
    import math

    iso = Iso()
    p = iso.p
    # Host adapter on the left (USB to TTL bus), its cable to the board
    iso.box(-7.6, -4.6, 2.4, 4.8, -0.4, 0.9, "#1b2a40", "#131f31", "#0e1726", "#2a4870")
    iso.box(-7.2, -6.2, 3.1, 4.1, 0.9, 1.05, "#c9d6ea", "#8aa0bf", "#6f86a6")
    iso.led(-5.2, 4.8, 0.6, "led-rx", r=2.4)
    bus = [p(-4.6, 3.6, 0.3), p(-2.0, 3.6, 0.3), p(0.0, 3.6, 0.3)]
    iso.path(bus, "#0e2a4f", 6)
    iso.path(bus, "#2f8cff", 2.0, ' opacity="0.9"')
    start, stop = p(-4.4, 3.6, 0.45), p(-0.2, 3.6, 0.45)
    track = f"M{start[0]:.1f},{start[1]:.1f} L{stop[0]:.1f},{stop[1]:.1f}"
    for color, glow, motion, visible in (("#2f8cff", "#9cc8ff", "0;1;1;1", "0;1;1;0;0;0"),
                                         ("#3fcf8e", "#a6f0cb", "1;1;1;0", "0;0;0;1;1;0")):
        cubes = "".join(f'<rect x="{-12 + k * 9}" y="-3.5" width="7" height="7" rx="1.5" '
                        f'fill="{color}" stroke="{glow}" stroke-width="0.8" '
                        f'transform="skewY(30)"/>' for k in range(3))
        iso.parts.append(
            f'<g opacity="0" filter="url(#glow)">{cubes}'
            f'<animateMotion dur="5s" repeatCount="indefinite" path="{track}" '
            f'keyPoints="{motion}" keyTimes="0;0.35;0.55;1" calcMode="linear"/>'
            f'<animate attributeName="opacity" dur="5s" repeatCount="indefinite" '
            f'values="{visible}" keyTimes="0;0.05;0.33;0.4;0.95;1"/></g>')

    # Controller board with the library chip
    iso.box(0, 10, 0, 7.2, -0.4, 0, "url(#boardTop)", "#0a1628", "#071122", "#1e3a66", 1.2)
    for i in range(6):
        y = 2.3 + i * 0.55
        iso.path([p(2.0, y), p(1.1, y), p(0.7, y - 0.4), p(0.25, y - 0.4)], "#1d5fbf", 1.3,
                 ' opacity="0.7"')
        x = 2.3 + i * 0.55
        iso.path([p(x, 2.0), p(x, 1.1), p(x + 0.35, 0.75), p(x + 0.35, 0.25)], "#1d5fbf", 1.3,
                 ' opacity="0.7"')
    trace = [p(5.4, 3.6), p(7.0, 3.6), p(9.2, 3.6)]
    iso.path(trace, "#1d5fbf", 1.6)
    iso.path(trace, "#5ea6ff", 1.6, ' class="flow"')
    for k in range(4):  # power stage
        x = 6.6 + (k % 2) * 1.0
        y = 4.6 + (k // 2) * 1.0
        iso.box(x, x + 0.7, y, y + 0.7, 0, 0.35, "#14243c", "#0c182b", "#09121f", "#2a4870")
    iso.box(9.2, 10.0, 2.4, 4.8, 0, 0.55, "#c9d6ea", "#8aa0bf", "#6f86a6")
    for i in range(7):
        c = 2.25 + i * 0.45
        iso.box(c - 0.08, c + 0.08, 5.2, 5.45, 0, 0.12, "#8fa6c6", "#6b819f", "#5a6f8b")
        iso.box(5.2, 5.45, c - 0.08, c + 0.08, 0, 0.12, "#8fa6c6", "#6b819f", "#5a6f8b")
    iso.box(2.0, 5.2, 2.0, 5.2, 0, 0.75, "url(#chipTop)", "#0a1322", "#060d18", "#2f8cff", 1.0)
    iso.text(3.6, 3.55, 0.76, HERO_CHIP[0], 0.42, "#e6edf7", 800, 0.03)
    iso.text(3.6, 4.25, 0.76, HERO_CHIP[1], 0.2, "#5ea6ff", 600, 0.04)
    iso.led(2.45, 2.45, 0.76)

    # Stepper motor on the right, two phase currents on its cable
    dx0, dx1, dy0, dy1 = 16.0, 19.6, 1.8, 5.4
    cable = [p(10.0, 3.6, 0.3), p(12.0, 3.6, 0.3), p(dx0, 3.6, 0.3)]
    iso.path(cable, "#0e2a4f", 6)
    iso.path(cable, BLUE, 2.2)
    for phase, color in ((0.0, "#5ea6ff"), (0.25, "#3fcf8e")):
        wave = [p(10.6 + t * (dx0 - 11.2), 3.6,
                  1.5 + 0.55 * math.sin(2 * math.pi * (2.5 * t + phase)))
                for t in (n / 100 for n in range(101))]
        iso.path(wave, color, 2.4, ' opacity="0.45" filter="url(#glow)"')
        iso.path(wave, color, 1.8, ' class="flow"')
    iso.box(dx0, dx1, dy0, dy1, -0.4, 2.6, "#1b2a40", "#131f31", "#0e1726", "#2a4870")
    for z in (0.3, 0.9, 1.5, 2.1):  # cooling fins of a stepper body
        iso.box(dx0 - 0.05, dx1 + 0.05, dy0 - 0.05, dy1 + 0.05, z, z + 0.08, "#24395a",
                "#1b2c46", "#15233a")
    iso.disc((dx0 + dx1) / 2, (dy0 + dy1) / 2, 2.6, 1.1,
             [(1.0, "#8aa0bf"), (0.88, "#c9d6ea"), (0.3, "#8aa0bf")])
    iso.led(dx1 - 0.5, dy1, 2.0, r=2.6)
    path = os.path.join(OUT, "hero-3d.svg")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(iso.svg("A host sends DYNAMIXEL Protocol 2.0 packets to a controller board; "
                        "the X430 emulator answers as an X430 and drives a stepper motor",
                        HERO_DEFS, HERO_STYLE, glow_at=(11, 4)))


def main() -> None:
    os.makedirs(OUT, exist_ok=True)
    hero()
    architecture()
    state_machine()
    integration()
    write_pipeline()
    fast_read()
    profile()
    for name in sorted(os.listdir(OUT)):
        if name.endswith(".svg"):
            print("wrote", os.path.join("docs", "assets", "images", name))


if __name__ == "__main__":
    main()
