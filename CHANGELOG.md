# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

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

[Unreleased]: https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/AntonVynohradov/DYNAMIXEL-X430-Control-Table-Emulator/releases/tag/v1.0.0
