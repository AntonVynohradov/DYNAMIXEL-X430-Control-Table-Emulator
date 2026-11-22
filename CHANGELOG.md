# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [1.2.0] - 2026-11-22

Less RAM per bus with DXL2-C 2.0.0. The public API is unchanged; the instance of a bus is
smaller, so code built against 1.1.0 must be recompiled.

### Changed

- DXL2-C 2.0.0 (was 1.0.0, `X430_DXL2_GIT_TAG`): the parser removes the byte stuffing in
  place, and the decoded packet points into its receive buffer instead of holding a copy
  of the parameters. `X430_DXL2_LINK_BYTES` follows the new `struct dxl2_link`.
- `DXL2_MAX_PACKET_SIZE` has one default per build system: `Kconfig.x430` and
  `x430.cmake` (768). `x430_config.h` no longer sets its own default; a build without
  Kconfig, without `x430.cmake` and without the definition stops with `#error` instead of
  using a size DXL2-C may not have been compiled with.

### Quality

- Cortex-M4 (arm-none-eabi-gcc 13.2.1, -Os, XM430-W350): 1,664 B RAM per bus with
  768-byte packets (was 2,424 B), 640 B with 256-byte packets (was 888 B); 384 B per
  device and 32 B per simulator as before. Flash 10,119 B with the model table (was
  10,191 B), DXL2-C 842 B (was 850 B). Worst-case stack unchanged (424 B,
  x430_bus_receive()).
- The unit tests (138 CTest tests, 105 without the optional features) with 100 % line
  and branch coverage of src/, all 17 models, the DynamixelSDK end-to-end check and MISRA C:2012
  (0 violations, Cppcheck 2.13) pass with DXL2-C 2.0.0.

## [1.1.0] - 2026-11-15

Several emulator buses on one wire: one bus per MCU, connected to one half-duplex
DYNAMIXEL bus (for example one stepper drive per joint), now behave like X series
actuators on that wire.

### Changed

- SYNC_READ and BULK_READ take turns: a hosted device answers only after the status
  packet of the ID listed just before it in the request (at once after a device of the
  same bus). The bus now decodes the status packets of other devices on the wire for
  this. Before, every hosted device answered at once and IDs of other buses were
  skipped, so the answers of several buses collided on a shared wire.
- If that status does not arrive within X430_TURN_TIMEOUT_MS, or the host sends its next
  instruction first, or a hosted device does not answer reads (Status Return Level 0),
  the remaining hosted devices of the read stay silent.
- FAST_SYNC_READ and FAST_BULK_READ: when the request lists an ID that the bus does not
  host, the bus stays silent (one bus cannot build the chained answer of several), and
  the ROBOTIS ROS 2 hardware interface falls back to SYNC_READ. Before, the bus answered
  for its own IDs only, which the host decoded as a malformed answer. With all IDs on one
  bus nothing changes.
- struct x430_bus grows by the pending read: 2,424 B on Cortex-M4 with the default
  options (was 2,368 B); x430_bus.c 3,090 B flash at -Os (was 2,820 B). The stack of
  every public function is unchanged. Rebuild the application with the new headers.

### Added

- X430_TURN_TIMEOUT_MS / CONFIG_X430_TURN_TIMEOUT_MS (default 100 ms, 1..60000): the
  longest wait for the status of the ID before, from the last progress of the read.

## [1.0.0] - 2026-11-14

First release: device-side emulation of DYNAMIXEL X430 actuators over Protocol 2.0. The
public API (x430.h and inc/) is stable within major version 1.

### Added

- Bus (struct x430_bus): decodes the instruction packets of one serial line with DXL2-C
  1.0.0 and answers for up to X430_MAX_DEVICES devices. PING, READ, WRITE, REG_WRITE,
  ACTION, FACTORY_RESET, REBOOT, CLEAR, BACKUP, SYNC_READ/WRITE, BULK_READ/WRITE,
  FAST_SYNC_READ and FAST_BULK_READ; broadcast, Secondary ID, Status Return Level, Return
  Delay Time, Bus Watchdog. x430_bus_handle_packet() for applications with their own
  DXL2-C link.
- Device (struct x430_device): the Control Table of the model selected at build time with
  the rules of the real actuator: access rights, ranges and limit registers, EEPROM lock
  while the torque is on, all-or-nothing checks of multi-register writes, Indirect
  Address/Data, hardware error shutdown and alert bit, Startup Configuration, Drive Mode
  "torque on by goal update", Goal Position := Present Position on torque on, factory
  reset, reboot, multi-turn clear and Control Table backup.
- Application hooks per device (write, read, event, nvm_read, nvm_write) and an optional
  platform port (lock, millisecond clock, delay). Persistent blocks carry a header (model,
  format, slot, size) and a CRC-16; a foreign or damaged block is ignored. The device reads
  and writes them in parts at an offset and keeps no copy of a block in RAM.
- Memory layout for small MCUs: a device keeps a compact image of its Control Table
  (316 of 662 bytes: the long reserved runs are left out); the bus builds every status
  packet in one buffer and stuffs it in place (no packet buffer on the stack); the model
  table has no address map and generates the 56 Indirect Address registers from the
  indirect blocks. A status whose byte-stuffed form exceeds DXL2_MAX_PACKET_SIZE is
  answered with a Data Length Error.
- All 17 X430 models (XM430-W350/W210, XH430-W350/W210/V350/V210, XD430-T350/T210,
  XW430-T333/T200, XL430-W250, 2XL430-W250, XC430-W150/W240/T150BB/T240BB, 2XC430-W250):
  Control Tables generated from models/*.yaml and checked against the ROBOTIS e-Manual
  tables. Application overrides of the model (X430_MODEL_OVERRIDES /
  CONFIG_X430_MODEL_OVERRIDES), for example a supply above 16 V. The generator needs only
  the Python standard library (PyYAML is used when installed).
- Simulator (struct x430_sim, x430_sim_hooks): a kinematic model with the X430 velocity
  profiles for tests and for setting up a system without hardware; POSIX host simulator
  examples/host_sim serving simulated devices on a serial port or pseudo-terminal.
- Configuration with plain compiler definitions (COMPILE_DEFINITIONS) or, when
  X430_KCONFIG is switched on, Kconfig (Kconfiglib, menuconfig); optional features BULK,
  FAST_READ, BACKUP and SIMULATOR; COMPILE_OPTIONS for the flags of the library sources.
  Packets of up to 768 bytes by default, so a host can read the whole Control Table at
  once (DYNAMIXEL Wizard).
- UNIT_TESTS builds the unit tests also when the library is a submodule of another
  project (executable x430_gtest, CTest label x430); the x430 library never contains them.
- Opaque, caller-owned instances; every function returns X430_OK or an X430_ERR_* code
  (PARAM, STATE, CONFIG, HAL, UNSUPPORTED) and refuses an instance without a successful
  initialization. Protocol errors go to the host in the status packet.
- Documentation website and PDF user manual: overview, architecture, usage, the emulated
  device in detail, integration (UART, RTOS, flash, ROS 2, DYNAMIXEL Wizard), API
  reference, MISRA deviation register.

### Quality

- Cortex-M4 (arm-none-eabi-gcc 13.2.1, -Os, XM430-W350, 768-byte packets): 384 B RAM
  per device, 2,368 B per bus (832 B with 256-byte packets), 32 B per simulator;
  9,921 B flash with the model table (+850 B DXL2-C); .data and .bss empty; worst-case
  stack 424 B (x430_bus_receive(), with any packet size).
- 131 GoogleTest cases with 100 % line and branch coverage of src/, and 100 cases with
  100 % without the optional features; unit tests for all 17 models; model data against
  the e-Manual; 52 checks end to end with the official ROBOTIS DynamixelSDK; the
  simulator's trapezoidal move within 0.7 % of the closed form.
- MISRA C:2012: 0 violations with Cppcheck 2.13, deviations DEV-01 (Rule 11.3, opaque
  instances) and DEV-02 (Rule 11.5, callback contexts). No warnings with -Wall -Wextra
  -Wpedantic -Wconversion; AddressSanitizer and UndefinedBehaviorSanitizer clean.
- Verified on hardware: a stepper joint firmware (STM32G431, Zephyr) emulating an
  XM430-W350 with DYNAMIXEL Wizard 2.0.

[Unreleased]: https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/compare/v1.2.0...HEAD
[1.2.0]: https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/releases/tag/v1.0.0
